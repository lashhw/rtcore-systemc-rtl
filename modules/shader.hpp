#ifndef RTCORE_SYSTEMC_SHADER_HPP
#define RTCORE_SYSTEMC_SHADER_HPP

template<int width, int height>
SC_MODULE(SHADER) {
    // ports
    sc_in<bool> s_valid;
    sc_out<bool> s_ready;
    sc_in<int> s_ray_id;
    sc_in<bool> s_hit;
    sc_in<int> s_hit_trig_idx;
    sc_in<float> s_t;
    sc_in<float> s_u;
    sc_in<float> s_v;

    sc_in<bool> clk;
    sc_in<bool> srstn;

    // high-level objects
    Bvh *bvh;
    std::unordered_map<int, int> *ray_id_to_pixel_idx;
    int framebuffer_r[width * height];
    int framebuffer_g[width * height];
    int framebuffer_b[width * height];

    SC_HAS_PROCESS(SHADER);
    SHADER(const sc_module_name &mn, Bvh *bvh, std::unordered_map<int, int> *ray_id_to_pixel_idx)
        : sc_module(mn), bvh(bvh), ray_id_to_pixel_idx(ray_id_to_pixel_idx) {
        SC_METHOD(main)
        sensitive << clk.pos();
        dont_initialize();

        SC_METHOD(update_s_ready)
        sensitive << srstn;
    }

    void main() {
        if (s_valid && s_ready) {
            int pixel_idx = (*ray_id_to_pixel_idx)[s_ray_id];
            if (s_hit) {
                float r = bvh->triangles[s_hit_trig_idx].n.x;
                float g = bvh->triangles[s_hit_trig_idx].n.y;
                float b = bvh->triangles[s_hit_trig_idx].n.z;
                float length = sqrtf(r * r + g * g + b * b);
                r = (r / length + 1.f) / 2.f;
                g = (g / length + 1.f) / 2.f;
                b = (b / length + 1.f) / 2.f;
                framebuffer_r[pixel_idx] = std::clamp(int(256.f * r), 0, 255);
                framebuffer_g[pixel_idx] = std::clamp(int(256.f * g), 0, 255);
                framebuffer_b[pixel_idx] = std::clamp(int(256.f * b), 0, 255);
            } else {
                framebuffer_r[pixel_idx] = 0;
                framebuffer_g[pixel_idx] = 0;
                framebuffer_b[pixel_idx] = 0;
            }
        }
    }

    void update_s_ready() {
        s_ready = srstn;
    }

    ~SHADER() {
        std::ofstream image_file("image.ppm");
        image_file << "P3\n" << width << ' ' << height << "\n255\n";
        for(int i = 0; i < height * width; i++) {
            image_file << framebuffer_r[i] << ' ' << framebuffer_g[i] << ' ' << framebuffer_b[i] << "\n";
        }
        image_file.close();
    }
};

#endif //RTCORE_SYSTEMC_SHADER_HPP