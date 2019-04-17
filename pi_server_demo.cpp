
/* ======================================================================== */
/*   pi_server.c                                                            */
/*   MPI program for calculating Pi by Monte Carlo estimation               */
/*   with separate RNG - basic code framework                               */
/* ======================================================================== */
/***
   * Stud. lab.206 MPICH compilation: 
   *   /opt/nfs/mpich-3.2/bin/mpicc -o pi_server pi_server.c -lm
   * Stud. lab.206 MPICH example execution:
   *   /opt/nfs/mpich-3.2/bin/mpiexec -n 4 ./pi_server 0.000001
   */

#include  <stdio.h>
#include  <stdlib.h> 
#include  <math.h> 
#include "mpi.h"
#include "sprng_cpp.h"
#define CHUNKSIZE 10000
#define SEED 985456376
/* We'd like a value that gives the maximum value returned by the function
   random, but no such value is *portable*.  RAND_MAX is available on many 
   systems but is not always the correct value for random (it isn't for 
   Solaris).  The value ((unsigned(1)&lt;&lt;31)-1) is common but not guaranteed */
#define THROW_MAX 100000000
#define PI 3.141592653589793238462643
/* message tags */
#define REQUEST 1
#define REPLY 2

int main(int argc, char *argv[])
{
    int numprocs, myid, server, workerid, ranks[1],
        request, i, iter, ix, iy, done;
    long rands[CHUNKSIZE], max, in, out, totalin, totalout;
    double x, y, Pi, error, epsilon;

    int streamnum, nstreams;
  Sprng *stream;
  double rn;

    MPI_Comm world, workers;
    MPI_Group world_group, worker_group;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    world = MPI_COMM_WORLD;
    MPI_Comm_size(world, &numprocs);
    MPI_Comm_rank(world, &myid);

    /***
   * Now Master should read epsilon from command line
   * and distribute it to all processes.
   */

    /* ....Fill in, please.... */

    if (myid == 0) // Read epsilon from command line
        sscanf(argv[1], "%lf", &epsilon);
    MPI_Bcast(&epsilon, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    /***
   * Create new process group called world_group containing all 
   * processes and its communicator called world
   * and a group called worker_group containing all processes
   * except the last one (called here server) 
   * and its communicator called workers.
   */

    /* ....Fill in, please.... */

    /*  Use MPI group and commmunicator operations....
    MPI_Comm_group( ... );
    ranks[0] = server;
    MPI_Group_excl( ... );
    MPI_Comm_create( ... );
    MPI_Group_free( ... );
    MPI_Group_free( ... );
*/

    /***
   * Server part
   *
   * Server should loop until request code is 0, in each iteration:
   * - receiving request code from any slave
   * - generating a vector of CHUNKSIZE randoms
   * - sending vector back to slave 
   */

    /***
   * Workers (including Master) part
   *
   * Worker should send initial request to server.
   * Later, in a loop worker should:
   * - receive vector of randoms
   * - compute x,y point inside unit square
   * - check (and count result) if point is inside/outside 
   *   unit circle
   * - sum both counts over all workers
   * - calculate Pi and its error (from "exact" value) in all workers
   * - test if error is within epsilon limit
   * - test continuation condition (error and max. points limit)
   * - print Pi by master only
   * - send a request to server (all if more or master only if finish)
   * Before finishing workers should free their communicator.
   */

    /* ....Fill in, please.... */

    /*
        MPI_Send( &amp;request, ... );
                        
            ... Loop until error &lt; epsilon or total &gt; THROW_MAX
                                                
            MPI_Recv( rands, ... );

            ... throw number of darts 
            ... calculate Pi globally
            ... test epsilon condition
            ... decide, should server be closed?
            MPI_Send( &amp;request, ...);
                                                            
	MPI_Comm_free( ... );
*/
    streamnum = myid;
    nstreams = numprocs;
    stream = SelectType(1);
    stream->init_sprng(streamnum, nstreams, SEED, SPRNG_DEFAULT);
    request = 1;
    done = 0;
    in = out = 0;
    max = RAND_MAX; // max int, for normalization
    iter = 0;
    while (!done)
    {
        iter++;
      
        
       
            x = stream->sprng() * 2 - 1;
            y = stream->sprng() * 2 - 1;
            if (x * x + y * y < 1.0)
            {
                in++;
            }
            else
                out++;
      

        // Workers update total statistics
        MPI_Allreduce(&in, &totalin, 1, MPI_LONG, MPI_SUM, MPI_COMM_WORLD);
        MPI_Allreduce(&out, &totalout, 1, MPI_LONG, MPI_SUM, MPI_COMM_WORLD);
        // Testing for final results
        Pi = (4.0 * totalin) / (totalin + totalout);
        error = fabs(Pi - PI);
        done = (error < epsilon || (totalin + totalout) > THROW_MAX);
        
        // Master must send next request, other workers may not
        if (myid == 0)
        {
            printf("\rpi = %23.20f", Pi);
        }
    }

    // Workers no longer needed

    /***
   * Master should print final point counts.
   */

    /* ....Fill in, please.... */

    if (myid == 0)
    {
        printf("\npoints: %d\nin: %d, out: %d, <ret> to exit\n",
               totalin + totalout, totalin, totalout);
        getchar();
    }

    /***
   * End of MPI operations.
   */
 stream->free_sprng();
    MPI_Finalize();
}