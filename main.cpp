#include "color.h"
#include "hittable.h"
#include "sphere.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "interval.h"
#define AMBIENT_STRENGTH 0.5f
#define DIFFUSE_STRENGTH 0.5f
#define MAX_DEPTH 5
#include <math.h>
#include <iostream>

color ray_color(const ray& r, const hittable& world, int depth) {
    hit_record rec;
    //vec3 lightDir = unit_vector(vec3(1.5, 0.5, -1));
    if (world.hit(r, interval(0.001, INFINITY), rec) && depth <= MAX_DEPTH) {
        vec3 direction = random_on_hemisphere(rec.normal);
        vec3 lightDir = unit_vector(vec3(1.5, 0.5, -1) - rec.p);
        //float coeff = -1 * dot(r.direction(), rec.normal);
        //return (rec.col * AMBIENT_STRENGTH) + ((-1 * dot(r.direction(), rec.normal)) * rec.col);
        
        float d = std::max(dot(lightDir, direction), 0.0);
        //std::cout << d <<"\n";
        return (rec.col * AMBIENT_STRENGTH*d) + (DIFFUSE_STRENGTH * ray_color(ray(rec.p, direction), world, depth + 1));
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5*(unit_direction.y() + 1.0);
    return color(0, 0, 0);
    //return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
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

    world.add(make_shared<sphere>(point3(1.5, 0.5, -1), 0.1, color(1,1,1)));

    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5, color(1,0,0)));
    world.add(make_shared<sphere>(point3(1, 0, -1), 0.2, color(0,1,0))); //green
    world.add(make_shared<sphere>(point3(0,-100.5,-1), 100, color(1,0,1))); //ground

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
    //set light direction..
    
    // Create a buffer to store the pixel data
    std::vector<unsigned char> image_data(image_width * image_height * 3);

    // Render
    for (int j = 0; j < image_height; ++j) {
        std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {
            color pixel_color(0, 0, 0);
            for (int sample = 0; sample < samples_per_pixel; ++sample) {
                // Get ray
                auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
                double random1 = rand() / (RAND_MAX + 1.0);
                double random2 = rand() / (RAND_MAX + 1.0);
                auto pixel_sample_square = ((-0.5 + random1) * pixel_delta_u) + ((-0.5 + random2) * pixel_delta_v);
                auto pixel_sample = pixel_center + pixel_sample_square;
                auto ray_direction = pixel_sample - camera_center;
                ray r(camera_center, ray_direction);

                int depth = 0;
                color objColor = ray_color(r, world, depth);

                pixel_color += objColor;
                //pixel_color += AMBIENT_STRENGTH * objColor; //adding ambience (?)
            }

             //pixel_color += AMBIENT_STRENGTH * pixel_color;
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