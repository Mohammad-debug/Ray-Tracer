#include "color.h"
#include "hittable.h"
#include "sphere.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "interval.h"
#define AMBIENT_STRENGTH 0.2f
#define DIFFUSE_STRENGTH 0.3f
#define SPECULAR_STRENGTH 0.6f
#define MIRROR_STRENGTH 0.5f
#define SHININESS 10000.0f
#define MAX_DEPTH 50
#include <iostream>
#include <string>
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

color ray_color(const ray &r, const hittable &world, int depth)
{
    hit_record rec;
    vec3 lighSource = vec3(1, 1, 1);
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
        if (world.hit(seconadry_ray, interval(0.001, INFINITY), rec) && depth <= MAX_DEPTH)
            return ambient;
        // diffuse
        vec3 diffuse = DIFFUSE_STRENGTH * std::max(dot(rec.normal, lightDir), 0.0) * rec.col;

        // specular

        vec3 refl_direction = reflect_half(rec.normal, lightDir);
        float shininess = pow(std::max(dot(rec.normal, refl_direction), 0.0), SHININESS);
        vec3 specular = rec.col * shininess * SPECULAR_STRENGTH;

        vec3 relected_ray = reflect(r.direction(), rec.normal);
        // if surface is glaze
        if (rec.glaze)
            return color(ambient + diffuse + specular) + MIRROR_STRENGTH * ray_color(ray(rec.p, unit_vector(relected_ray)), world, depth + 1);
        else
            return color(ambient + diffuse + specular);
    }

    // vec3 unit_direction = unit_vector(r.direction());
    // auto a = 0.5*(unit_direction.y() + 1.0);
    // return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
    return color(0, 0, 0);
}

void DisplayImage(const std::vector<unsigned char> &image_data, int image_width, int image_height)
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    // Create a windowed mode window and its OpenGL context
    GLFWwindow *window = glfwCreateWindow(800, 600, "Image Display", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        std::cerr << "Failed to create GLFW window" << std::endl;
        return;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Initialize GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cerr << "GLEW initialization error: " << glewGetErrorString(err) << std::endl;
        glfwTerminate();
        return;
    }
    // Flip the image vertically
    std::vector<unsigned char> flipped_image(image_data);
    for (int i = 0; i < image_height / 2; ++i)
    {
        int flip_i = image_height - 1 - i;
        for (int j = 0; j < image_width * 3; ++j)
        {
            std::swap(flipped_image[i * image_width * 3 + j], flipped_image[flip_i * image_width * 3 + j]);
        }
    }

    // Calculate the position to center the image
    int xPos = (800 - image_width) / 2;
    int yPos = (600 - image_height) / 2;

    // Main rendering loop
    while (!glfwWindowShouldClose(window))
    {
        // Render
        // Set the raster position to center the image
        //glRasterPos2i(xPos, yPos);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawPixels(image_width, image_height, GL_RGB, GL_UNSIGNED_BYTE, &flipped_image[0]);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up

    glfwTerminate();
}

int main()
{
    // Image

    auto aspect_ratio = 16.0 / 9.0;
    int image_width = 400;       // 800
    int samples_per_pixel = 100; // Count of random samples for each pixel
    auto scale = 1.0 / samples_per_pixel;
    bool orthogonal = false;
    // Calculate the image height, and ensure that it's at least 1.
    int image_height = static_cast<int>(image_width / aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;

    // World
    hittable_list world;
    // world.add(make_shared<sphere>(point3(1, 0.5, 1), 0.5, color(1, 1, 1)));
    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5, color(1, 0, 0), false));
    world.add(make_shared<sphere>(point3(1, -0.2, -1), 0.2, color(1, 1, 0), false));
    world.add(make_shared<sphere>(point3(0, -100.5, -1), 100, color(0.5, 0.5, 0.5), true)); // ground

    // Camera
    auto focal_length = 1.0;
    auto viewport_height = 2.0;
    auto viewport_width = viewport_height * (static_cast<double>(image_width) / image_height);

    // Create a buffer to store the pixel data
    std::vector<unsigned char> image_data(image_width * image_height * 3);
    // std::vector<unsigned char> image_data(image_width * image_height * 3);

    for (int t = 0; t < 1; t++)
    {
        auto camera_center = point3(0, t * 0.1, 1.5);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        auto viewport_u = vec3(viewport_width, 0, 0);
        auto viewport_v = vec3(0, -viewport_height, 0);

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        auto pixel_delta_u = viewport_u / image_width;
        auto pixel_delta_v = viewport_v / image_height;

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
                    orthogonal = true;
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
                    color objColor = ray_color(r, world, depth);

                    pixel_color += objColor;
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
        // std::string str = "frames/output_" + std::to_string(t) + ".png";
        std::string str = "output_" + std::to_string(t) + ".png";

        const char *charArray = str.c_str();
        stbi_write_png(charArray, image_width, image_height, 3, image_data.data(), image_width * 3);
        DisplayImage(image_data, image_width, image_height);
        std::clog << "\rDone. Image saved as " << str << "\n";
    }
}