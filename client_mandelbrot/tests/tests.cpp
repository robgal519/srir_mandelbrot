//
// Created by glaeqen on 06/04/19.
//

#include <gtest/gtest.h>
#include <DataRequestNamedPipe.h>
#include <DataResponseNamedPipe.h>

//struct Request {
//    int windowWidth;
//    int windowHeight;
//    double leftTopX;
//    double leftTopY;
//    double rightBottomX;
//    double rightBottomY;
//};


TEST(Test1, Test1){
    DataRequestNamedPipe request("/tmp/.treq");
    DataResponseNamedPipe response("/tmp/.tresp");
    int width = 500;
    int height = 500;

    request.sendRequest(Request{width, height, 1., 1., 1., 1.});
    std::vector<Pixel> image;
    response.readResponse(image, width, height);

    for(const auto& pixel : image){
        ASSERT_EQ(pixel.red, 0);
        ASSERT_EQ(pixel.green, 0xff);
        ASSERT_EQ(pixel.blue, 0);
    }
}
