///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017, STEREOLABS.
//
// All rights reserved.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////


#include <sl/Camera.hpp>

using namespace sl;

int main(int argc, char **argv) {

    // Create a ZED camera object
    Camera zed;

    // Set configuration parameters
    InitParameters init_params;
    init_params.depth_mode = DEPTH_MODE_PERFORMANCE; // Use PERFORMANCE depth mode
    init_params.coordinate_units = UNIT_MILLIMETER; // Use millimeter units (for depth measurements)


    // Open the camera
    ERROR_CODE err = zed.open(init_params);
    if (err != SUCCESS)
        exit(-1);

    // Set runtime parameters after opening the camera
    RuntimeParameters runtime_parameters;
    runtime_parameters.sensing_mode = SENSING_MODE_STANDARD; // Use STANDARD sensing mode

    // Capture 50 images and depth, then stop
    int i = 0;
    sl::Mat image, depth;
    while (i < 50) {
        // Grab an image
        if (zed.grab(runtime_parameters) == SUCCESS) {

            // A new image is available if grab() returns SUCCESS
            zed.retrieveImage(image, VIEW_LEFT); // Get the left image
            zed.retrieveMeasure(depth, MEASURE_DEPTH); // Retrieve depth Mat. Depth is aligned on the left image

            // Get and print depth value in mm at the center of the image
            int x = image.getWidth() / 2;
            int y = image.getHeight() / 2;
            float depth_value = 0.f;
            depth.getValue(x, y, &depth_value);
            printf("Depth at (%d, %d): %f mm\n", x, y, depth_value);
            i++;
        }
    }

    // Close the camera
    zed.close();
    return 0;
}
