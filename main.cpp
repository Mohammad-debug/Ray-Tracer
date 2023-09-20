#include "color.h"
#include "hittable.h"
#include "sphere.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "interval.h"

#include <iostream>

double random_double(double min, double max) {
    return min + (max - min) * (rand() / (RAND_MAX + 1.0));
}

color ray_color(const ray& r, const hittable& world) {
    hit_record rec;
    if (world.hit(r, interval(0, INFINITY), rec)) {
        return 0.5 * (rec.normal + color(1, 1, 1));
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5*(unit_direction.x() + 1.0);
    return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
}



int main() {
    // Image

    auto aspect_ratio = 16.0 / 9.0;
    int image_width = 400;
    int samples_per_pixel = 100; // Count of random samples for each pixel
    auto scale = 1.0 / samples_per_pixel;

    // Calculate the image height, and ensure that it's at least 1.
    int image_height = static_cast<int>(image_width / aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;

    // World
    hittable_list world;

    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
    //world.add(make_shared<sphere>(point3(0,-100.5,-1), 100)); //ground

    // Camera

    auto focal_length = 1.0;
    auto viewport_height = 2.0;
    auto viewport_width = viewport_height * (static_cast<double>(image_width) / image_height);
    auto camera_center = point3(0, 0, 0);

    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    auto viewport_u = vec3(viewport_width, 0, 0);
    auto viewport_v = vec3(0, -viewport_height, 0);

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    auto pixel_delta_u = viewport_u / image_width;
    auto pixel_delta_v = viewport_v / image_height;

    // Calculate the location of the upper left pixel.
    auto viewport_upper_left = camera_center - vec3(0, 0, focal_length) - viewport_u / 2 - viewport_v / 2;
    auto pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

    // Create a buffer to store the pixel data
    std::vector<unsigned char> image_data(image_width * image_height * 3);

    // Render
    for (int j = 0; j < image_height; ++j) {
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {
            auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
            auto ray_direction = pixel_center - camera_center;
            ray r(camera_center, ray_direction);
            color pixel_color = ray_color(r);

            //Write color
            static const interval intensity(0.000, 0.999);
            // Convert the color to integer values in the range [0, 255]
            int ir = static_cast<int>(255.999 * intensity.clamp(scale * pixel_color.x()));
            int ig = static_cast<int>(255.999 * intensity.clamp(scale * pixel_color.y()));
            int ib = static_cast<int>(255.999 * intensity.clamp(scale * pixel_color.z()));

            // Calculate the index in the image_data array
            int index = 3 * (j * image_width + i);

            // Set the RGB values in the image_data array
            image_data[index] = static_cast<unsigned char>(ir);
            image_data[index + 1] = static_cast<unsigned char>(ig);
            image_data[index + 2] = static_cast<unsigned char>(ib);
        }
    }

    // Save the image as a PNG file using stb_image_write
    stbi_write_png("output.png", image_width, image_height, 3, image_data.data(), image_width * 3);

    std::clog << "\rDone. Image saved as 'output.png'\n";
}
