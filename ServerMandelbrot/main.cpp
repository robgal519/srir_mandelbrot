/*
 * Calculate Pi using Monte Carlo method.
 *
 * This program demonstrates different UPC++ communication styles, 
 * shown in reduce_to_rank0_* functions.
 * For more details, see UPC++ Programmer's Guide (ver. 2019.3.0):
 * https://bitbucket.org/berkeleylab/upcxx/downloads/upcxx-guide-2019.3.0.pdf
 * 
 * PG 04/2019
 * 
 * Setup:
  $ source /opt/nfs/config/source_upcxx.sh 

 * Compile (using UDP transport):
  $ UPCXX_GASNET_CONDUIT=udp upcxx -O2 upcxx-compute-pi-other.cpp -o upcxx-compute-pi-other

 * Run 8 tasks (ranks) on cluster defined in 'nodes' file with 10 million throws each
  $ upcxx-run -n 8 $(upcxx-nodes nodes) ./upcxx-compute-pi-other 10000000 2>/dev/null

 *
 */

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <random>
#include <sys/time.h>
#include <upcxx/upcxx.hpp>

using namespace std;

// Timing operations
double tstart, tend;

double wctime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec + 1E-6 * tv.tv_usec);
}


// Choose a (x,y) point at random
double x, y;				
unsigned short xsubi[3];		// RNG: last 48-bit Xi generated
struct drand48_data buffer;		// RNG: state buffer

// std::int64_t is used to prevent overflows
int64_t hit()
{
    erand48_r(xsubi, &buffer, &x);
    erand48_r(xsubi, &buffer, &y);
    if (x*x + y*y <= 1.0) return 1; 	// hit
    else return 0;			// miss
}


///////// 1: sum the hits to rank 0 using reduce_all /////////
 
int64_t reduce_to_rank0(int64_t my_hits)
{
    // Wait for a collective reduction that sums all local values,
    // reduce_all - introduced in version 2019.3.0, 
    // "plus<int64_t>()" instead of "upcxx::op_fast_add" still works
    return upcxx::reduce_all(my_hits, upcxx::op_fast_add).wait();
}


// Need to declare some global variables to use with RPC and other examples
int64_t hits_counter = 0;
int64_t hits = 0;

///////// 2: sum the hits to rank 0 using global_ptr /////////

int64_t reduce_to_rank0_global(int64_t my_hits)
{
    // Rank 0 creates an array the size of the number of ranks to store all
    // the global pointers
    upcxx::global_ptr<int64_t> all_hits_ptr = nullptr;
    if (upcxx::rank_me() == 0) {
      all_hits_ptr = upcxx::new_array<int64_t>(upcxx::rank_n());
    }
    // Rank 0 broadcasts the array global pointer to all ranks
    all_hits_ptr = upcxx::broadcast(all_hits_ptr, 0).wait();
    // All ranks offset the start pointer of the array by their rank to point
    // to their own chunk of the array
    upcxx::global_ptr<int64_t> my_hits_ptr = all_hits_ptr + upcxx::rank_me();
    // Every rank now puts its own hits value into the correct part of the array
    upcxx::rput(my_hits, my_hits_ptr).wait();		// remote memory access
    upcxx::barrier();					// synchronization
    // Now rank 0 gets all the values stored in the array
    int64_t hits = 0;
    if (upcxx::rank_me() == 0) {
      // get a local pointer to the shared object on rank 0
      int64_t *local_hits_ptrs = all_hits_ptr.local();
      for (int i = 0; i < upcxx::rank_n(); i++) {
        hits += local_hits_ptrs[i];
      }
      upcxx::delete_array(all_hits_ptr);
    }
    return hits;
}

///////// 3: sum the hits to rank 0 using rpc /////////

int64_t reduce_to_rank0_rpc(int64_t my_hits)
{
    int64_t expected_hits = upcxx::rank_n();
    // Wait for an rpc that updates rank 0's count
    upcxx::rpc(	0,				// target rank
                [](int64_t my_hits) { 		// lambda function
                                      hits += my_hits;
                                      hits_counter++;
                                    },
                my_hits				// lambda argument
              ).wait();				// wait for future completion
    // Wait until all ranks have updated the count (barrier replacement)
    if (upcxx::rank_me() == 0)
       while (hits_counter < expected_hits) upcxx::progress();
    // hits is only set for rank 0 at this point, which is OK because only
    // rank 0 will print out the result
    return hits;
}


///////// 4: sum the hits to rank 0 using dist_object  /////////

int64_t reduce_to_rank0_dist(int64_t my_hits)
{
    // Declare a distributed object on every rank
    upcxx::dist_object<int64_t> all_hits(0);
    // Set the local value of the distributed object on each rank
    *all_hits = my_hits;
    upcxx::barrier();
    int64_t hits = 0;
    if (upcxx::rank_me() == 0) {
      // Rank 0 gets all the values
      for (int i = 0; i < upcxx::rank_n(); i++) {
        // Fetch the distributed object from remote rank i
        hits += all_hits.fetch(i).wait();
        }
    }
    // Ensure that no distributed objects are destructed before rank 0 is done
    upcxx::barrier();
    return hits;
}


///////// 5: sum the hits to rank 0 using conjoined futures  /////////

int64_t reduce_to_rank0_conjoin(int64_t my_hits)
{
    // Initialize this rank's part of the distributed object with the local value
    upcxx::dist_object<int64_t> all_hits(my_hits);
    int64_t hits = 0;
    // Rank 0 gets all the values asynchronously
    if (upcxx::rank_me() == 0) {
      hits = my_hits;
      upcxx::future<> f = upcxx::make_future();  // Initial ready future
      for (int i = 1; i < upcxx::rank_n(); i++) { // Loop over remote ranks
        // Construct the conjoined futures
        f = upcxx::when_all( f,  // Returns a future with concatenated results tuple of the arguments
                             all_hits.fetch(i).then( // Callback on the initiating rank as a lambda function
                                                     // rhit is fetch result
                                                     [&](int64_t rhit) { hits += rhit; }
                                                   )
                           );
      }
      // Wait for the futures to complete
      f.wait();
    }
    upcxx::barrier();
    return hits;
}


///////// 6: sum the hits to rank 0 using atomics  /////////

int64_t reduce_to_rank0_atomics(int64_t my_hits)
{
    // Create the atomic domain with load and fetch_add operations
    upcxx::atomic_domain<int64_t> ad_i64({upcxx::atomic_op::load, upcxx::atomic_op::fetch_add});
    // a global pointer to the atomic counter in rank 0's shared segment
    upcxx::global_ptr<int64_t> hits_ptr =
        (!upcxx::rank_me() ? upcxx::new_<int64_t>(0) : nullptr);
    // Rank 0 allocates and then broadcasts the global pointer to all other ranks
    hits_ptr = upcxx::broadcast(hits_ptr, 0).wait();
    // Now each rank updates the global pointer value using atomics for correctness
    ad_i64.fetch_add(hits_ptr, my_hits, memory_order_relaxed).wait();
    // Wait until all ranks have updated the counter
    upcxx::barrier();
    // Once a memory location is accessed with atomics, it should only be
    // subsequently accessed using atomics to prevent unexpected results
    if (upcxx::rank_me() == 0) {
        hits = ad_i64.load(hits_ptr, memory_order_relaxed).wait();
    } else {
        hits = 0;
    }
    // Explicitly destroy atomic domain - needed since version 2019.3.0
    ad_i64.destroy();
    return hits;
}


///////// 7: sum the hits to rank 0 using fire and forget rpc /////////

// Counts the number of ranks for which the RPC has completed
int n_done = 0;

int64_t reduce_to_rank0_ff(int64_t my_hits)
{
    // Cannot wait for the RPC - there is no completion handler returned
    upcxx::rpc_ff(0, [](int64_t my_hits) { hits += my_hits; n_done++; }, my_hits);
    if (upcxx::rank_me() == 0) {
      // Spin waiting for RPCs from all ranks to complete
      // When spinning, call the progress function to
      // ensure rank 0 processes waiting RPCs
      while (n_done != upcxx::rank_n()) upcxx::progress();
    }
    // Wait until all RPCs have been processed (quiescence)
    upcxx::barrier();
    return hits;
}


///////// 8: sum the hits to rank 0 using completions  /////////

upcxx::promise<> prom;
upcxx::global_ptr<int64_t> all_hits_ptr = nullptr;

int64_t reduce_to_rank0_completions(int64_t my_hits)
{
    if (upcxx::rank_me() == 0)
      prom.require_anonymous(upcxx::rank_n()); // explicit promise to be fulfilled by all ranks
    if (upcxx::rank_me() == 0)
      all_hits_ptr = upcxx::new_array<int64_t>(upcxx::rank_n());
    // Rank 0 broadcasts the array global pointer to all ranks
    all_hits_ptr = upcxx::broadcast(all_hits_ptr, 0).wait();
    // All ranks offset the start pointer of the array by their rank to point
    // to their own chunk of the array
    upcxx::global_ptr<int64_t> my_hits_ptr = all_hits_ptr + upcxx::rank_me();
    // Multiple completion notifications from one operation using the pipe (|) operator
    // to combine completion objects
    auto cxs = (upcxx::operation_cx::as_future() | // Completion through the returned future
                upcxx::remote_cx::as_rpc( // RPC completion which fulfills a promise on rank 0
                                          [](upcxx::intrank_t rank) { // lambda function
                                                hits += *(all_hits_ptr + rank).local();
                                                prom.fulfill_anonymous(1); // fulfilled by one rank
                                          }, 
                                          upcxx::rank_me())); // lambda argument
    // All ranks try to write to their own part on rank 0 and then accumulate
    auto result = upcxx::rput(my_hits, my_hits_ptr, cxs);
    result.wait(); // Sender is notified of its completion through the returned future.
    upcxx::future<> fut = prom.finalize(); // When all ranks have executed their rput and the 
                             // associated RPC completion, 'prom' will be satisfied on rank 0 
    fut.wait(); 	     // and the associated future 'fut' will become ready.
    if (upcxx::rank_me() == 0)
      upcxx::delete_array(all_hits_ptr);
    return hits;
}

/////////////////////////////////////////////////////////////////////
////////////////////////// main function  ///////////////////////////
/////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    upcxx::init();
    // each rank gets its own copy of local variables
    int64_t my_hits = 0;
    // the number of trials to run on each rank
    int64_t my_trials = 1000000; // default 1e6 
    // each rank gets its own local copies of input arguments
    if (argc == 2) my_trials = atoi(argv[1]);
    // initialize the random number generator differently for each rank
    srand48_r(upcxx::rank_me()*1357111, &buffer);
    
    // do the computation
    for (int64_t i = 0; i < my_trials; i++) {
        my_hits += hit();
    }

    ///////////////////////////////////////////////////////////
    
    // warmup UPC++ communication layer
    hits = reduce_to_rank0(my_hits);

    ///////////////////////////////////////////////////////////
    
    // sum the hits and print out the final result
    //////////////// using reduce_all collective
    hits = 0; 
    tstart = wctime();
    hits = reduce_to_rank0(my_hits);
    tend = wctime();
    // only rank 0 prints the result
    if (upcxx::rank_me() == 0) {
        // the total number of trials over all ranks
        int64_t trials = upcxx::rank_n() * my_trials;

        cout << endl << "0: #### Pi estimated on rank " << upcxx::rank_me() << " alone: "  << fixed << setprecision(7)
	     << 4.0l * (double) my_hits / (double) my_trials << endl; 

        cout << endl << "1: #### Using reduce_all" << endl;
        cout << "Pi estimated on " << upcxx::rank_n() << " tasks: " << fixed << setprecision(7) 
	     << 4.0l * hits / trials << " , elapsed time [s]: " << tend - tstart  << endl; 
    }

    // sum the hits and print out the final result
    //////////////// using global pointer
    hits = 0;
    tstart = wctime();
    hits = reduce_to_rank0_global(my_hits);
    tend = wctime();
    // only rank 0 prints the result
    if (upcxx::rank_me() == 0) {
        // the total number of trials over all ranks
        int64_t trials = upcxx::rank_n() * my_trials;

        cout << endl << "2: #### Using global_ptr" << endl;
        cout << "Pi estimated on " << upcxx::rank_n() << " tasks: " << fixed << setprecision(7) 
	     << 4.0l * hits / trials << " , elapsed time [s]: " << tend - tstart <<  endl; 
    }

    // sum the hits and print out the final result
    //////////////// using RPC
    hits = 0;
    tstart = wctime();
    hits = reduce_to_rank0_rpc(my_hits);
    tend = wctime();
    // only rank 0 prints the result
    if (upcxx::rank_me() == 0) {
        // the total number of trials over all ranks
        int64_t trials = upcxx::rank_n() * my_trials;

        cout << endl << "3: #### Using rpc" << endl;
        cout << "Pi estimated on " << upcxx::rank_n() << " tasks: " << fixed << setprecision(7) 
	     << 4.0l * hits / trials << " , elapsed time [s]: " << tend - tstart  << endl; 
    }

    // sum the hits and print out the final result
    //////////////// using distributed object
    hits = 0;
    tstart = wctime();
    hits = reduce_to_rank0_dist(my_hits);
    tend = wctime();
    // only rank 0 prints the result
    if (upcxx::rank_me() == 0) {
        // the total number of trials over all ranks
        int64_t trials = upcxx::rank_n() * my_trials;

        cout << endl << "4: #### Using dist_object" << endl;
        cout << "Pi estimated on " << upcxx::rank_n() << " tasks: " << fixed << setprecision(7) 
	     << 4.0l * hits / trials << " , elapsed time [s]: " << tend - tstart  << endl; 
    }

    // sum the hits and print out the final result
    //////////////// using conjoined futures
    hits = 0;
    tstart = wctime();
    hits = reduce_to_rank0_conjoin(my_hits);
    tend = wctime();
    // only rank 0 prints the result
    if (upcxx::rank_me() == 0) {
        // the total number of trials over all ranks
        int64_t trials = upcxx::rank_n() * my_trials;

        cout << endl << "5: #### Using conjoined futures" << endl;
        cout << "Pi estimated on " << upcxx::rank_n() << " tasks: " << fixed << setprecision(7) 
	     << 4.0l * hits / trials << " , elapsed time [s]: " << tend - tstart  << endl; 
    }

    // sum the hits and print out the final result
    //////////////// using atomic operations
    hits = 0;
    tstart = wctime();
    hits = reduce_to_rank0_atomics(my_hits);
    tend = wctime();
    // only rank 0 prints the result
    if (upcxx::rank_me() == 0) {
        // the total number of trials over all ranks
        int64_t trials = upcxx::rank_n() * my_trials;

        cout << endl << "6: #### Using atomics" << endl;
        cout << "Pi estimated on " << upcxx::rank_n() << " tasks: " << fixed << setprecision(7) 
	     << 4.0l * hits / trials << " , elapsed time [s]: " << tend - tstart  << endl; 
    }

    // sum the hits and print out the final result
    //////////////// using "fire and forget" RPC
    hits = 0;
    tstart = wctime();
    hits = reduce_to_rank0_ff(my_hits);
    tend = wctime();
    // only rank 0 prints the result
    if (upcxx::rank_me() == 0) {
        // the total number of trials over all ranks
        int64_t trials = upcxx::rank_n() * my_trials;

        cout << endl << "7: #### Using fire and forget rpc" << endl;
        cout << "Pi estimated on " << upcxx::rank_n() << " tasks: " << fixed << setprecision(7) 
	     << 4.0l * hits / trials << " , elapsed time [s]: " << tend - tstart  << endl; 
    }

    // sum the hits and print out the final result
    ///////////////// using completion objects
    hits = 0;
    tstart = wctime();
    hits = reduce_to_rank0_completions(my_hits);
    tend = wctime();
    // only rank 0 prints the result
    if (upcxx::rank_me() == 0) {
        // the total number of trials over all ranks
        int64_t trials = upcxx::rank_n() * my_trials;

        cout << endl << "8: #### Using completions" << endl;
        cout << "Pi estimated on " << upcxx::rank_n() << " tasks: " << fixed << setprecision(7) 
	     << 4.0l * hits / trials << " , elapsed time [s]: " << tend - tstart  << endl; 
    }
    
    ////////////////////////////////////////////////

    upcxx::finalize();
    return 0;
}
