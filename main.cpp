#include "color.h"
#include "hittable.h"
#include "sphere.h"
#include "tetrahedral.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "interval.h"
#define AMBIENT_STRENGTH 0.3f
#define DIFFUSE_STRENGTH 0.4f
#define SPECULAR_STRENGTH 0.6f
#define MIRROR_STRENGTH 0.5f
#define SHININESS 10000.0f
#define MAX_DEPTH 50
#include <iostream>
#include <string>

color ray_color(const ray &r, const hittable &world, int depth, vec3 lighSource)
{
    hit_record rec;

    if (world.hit(r, interval(0.001, INFINITY), rec) && depth <= MAX_DEPTH)
    {
        // vec3 direction = random_on_hemisphere(rec.normal);

        vec3 lightDir = unit_vector(lighSource - rec.p);
        ray seconadry_ray = ray(rec.p, lightDir);
        if (world.hit(seconadry_ray, interval(0.001, INFINITY), rec) && depth <= MAX_DEPTH)
            return color(0, 0, 0);

        // vec3 direction = rec.normal + random_unit_vector();

        // ambient
        vec3 ambient = rec.col * AMBIENT_STRENGTH;

        // diffuse
        vec3 diffuse = DIFFUSE_STRENGTH * std::max(dot(rec.normal, lightDir), 0.0) * rec.col;

        // specular

        vec3 refl_direction = reflect_half(rec.normal, lightDir);
        float shininess = pow(std::max(dot(rec.normal, refl_direction), 0.0), SHININESS);
        vec3 specular = rec.col * shininess * SPECULAR_STRENGTH;

        vec3 relected_ray = reflect(r.direction(), rec.normal);
        // if surface is glaze
        if (rec.glaze)
        {
            vec3 c(ambient + diffuse + specular);
            c += MIRROR_STRENGTH * ray_color(ray(rec.p, unit_vector(relected_ray)), world, depth + 1, lighSource);

            if (c.x() > 1.0)
                c.e[0] = 1.0;
            if (c.y() > 1.0)
                c.e[1] = 1.0;
            if (c.z() > 1.0)
                c.e[2] = 1.0;
            return c;
        }
        else
        {
            vec3 c(ambient + diffuse + specular);
            if (c.x() > 1.0)
                c.e[0] = 1.0;
            if (c.y() > 1.0)
                c.e[1] = 1.0;
            if (c.z() > 1.0)
                c.e[2] = 1.0;
            return c;
        }
    }

    // vec3 unit_direction = unit_vector(r.direction());
    // auto a = 0.5*(unit_direction.y() + 1.0);
    // return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
    return color(0, 0, 0);
}

int main()
{
    // Image

    auto aspect_ratio = 16.0 / 9.0;
    int image_width = 400;     // 800
    int samples_per_pixel = 5; // Count of random samples for each pixel
    auto scale = 1.0 / samples_per_pixel;
    bool orthogonal = false;
    // Calculate the image height, and ensure that it's at least 1.
    int image_height = static_cast<int>(image_width / aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;

    // World
    hittable_list world;
    vec3 lighSource = vec3(-1, 6, 1);

    world.add(make_shared<tetrahedron>(point3(-3.5, 2, -1), point3(-4, -0.0, 0), point3(-3.5, -0.0, -1), point3(-2, -0.0, -1), color(0, 1, 1), false));
    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5, color(1, 0, 0), false));
    world.add(make_shared<sphere>(point3(1, -0.2, -1), 0.2, color(1, 1, 0), false));
    world.add(make_shared<sphere>(point3(0, -100.5, -1), 100, color(0.5, 0.5, 0.5), true)); // ground

    // Camera
    auto focal_length = 1.0;
    auto viewport_height = 2.0;
    auto viewport_width = viewport_height * (static_cast<double>(image_width) / image_height);

    // Create a buffer to store the pixel data
    std::vector<unsigned char> image_data(image_width * image_height * 3);

    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    auto viewport_u = vec3(viewport_width, 0, 0);
    auto viewport_v = vec3(0, -viewport_height, 0);

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    auto pixel_delta_u = viewport_u / image_width;
    auto pixel_delta_v = viewport_v / image_height;

    // Time settings
    const double animation_duration = 8.0; // Adjust as needed
    const int frames_per_second = 20;
    const int total_frames = static_cast<int>(animation_duration * frames_per_second);
    auto camera_center = point3(0, 0.5, 2);

    std::cout << "total frames" << total_frames << "\n";
    for (int frame = 0; frame < total_frames ; frame++)
    {
        // Calculate the current time within the animation duration
        double time = static_cast<double>(frame) / frames_per_second;

        // Update the camera_center position based on a sine function
        double angle = 2.0 * M_PI * time / animation_duration;
        double amplitude = 1.0; // Adjust as needed
        camera_center.e[0] = -2.5 + amplitude * std::sin(angle);
        camera_center.e[2] = 2.5 + amplitude * std::sin(angle);
        //world.objects[1].center.x = -2.5 + amplitude * std::sin(angle);
        // Calculate the location of the upper left pixel.
        auto viewport_upper_left = camera_center - vec3(0, 0, focal_length) - viewport_u / 2 - viewport_v / 2;
        auto pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        for (int j = 0; j < image_height; ++j)
        {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; ++i)
            {
                color pixel_color(0, 0, 0);
                for (int sample = 0; sample < samples_per_pixel; ++sample)
                {
                    // Get ray
                    auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
                    double random1 = rand() / (RAND_MAX + 1.0);
                    double random2 = rand() / (RAND_MAX + 1.0);
                    auto pixel_sample_square = ((-0.5 + random1) * pixel_delta_u) + ((-0.5 + random2) * pixel_delta_v);
                    auto pixel_sample = pixel_center + pixel_sample_square;
                    auto ray_direction = pixel_sample - camera_center;
                    orthogonal = false;
                    ray r;
                    if (orthogonal)
                    {
                        r = ray(pixel_sample, -vec3(0, 0, 1));
                    }
                    else
                    {
                        r = ray(camera_center, ray_direction);
                    }

                    int depth = 0;
                    color objColor = ray_color(r, world, depth, lighSource);

                    pixel_color += objColor;
                    // pixel_color += AMBIENT_STRENGTH * objColor; //adding ambience (?)
                }

                // pixel_color += AMBIENT_STRENGTH * pixel_color;
                // Write color
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
        std::string str = "frames/output_" + std::to_string(frame) + ".png";
        // std::string str = "output_" + std::to_string(t) + ".png";
        const char *charArray = str.c_str();
        stbi_write_png(charArray, image_width, image_height, 3, image_data.data(), image_width * 3);
        std::clog << "\rDone. Image saved as " << str << "\n";
    }
}