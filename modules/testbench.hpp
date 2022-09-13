#ifndef RTCORE_SYSTEMC_TESTBENCH_HPP
#define RTCORE_SYSTEMC_TESTBENCH_HPP

#include <fstream>
#include <unordered_map>
#include "rtcore/rtcore.hpp"
#include "rtcore/post.hpp"

SC_MODULE(TESTBENCH) {
    // parameters
    static constexpr float origin_x = 0.f;
    static constexpr float origin_y = 0.1f;
    static constexpr float origin_z = 1.f;
    static constexpr float horizontal = 0.2f;
    static constexpr float vertical = 0.2f;
    static constexpr int width = 600;
    static constexpr int height = 600;

    // submodules
    RTCORE<4> rtcore;

    // high-level objects
    Bvh *bvh;
    int framebuffer_r[height * width];
    int framebuffer_g[height * width];
    int framebuffer_b[height * width];
    std::unordered_map<int, int> ray_id_to_pixel_idx;

    // internal signals
    sc_signal<bool> m_valid;
    sc_signal<bool> m_ready;
    sc_signal<float> m_origin_x;
    sc_signal<float> m_origin_y;
    sc_signal<float> m_origin_z;
    sc_signal<float> m_dir_x;
    sc_signal<float> m_dir_y;
    sc_signal<float> m_dir_z;
    sc_signal<float> m_tmax;
    sc_signal<int> m_ray_id;

    sc_clock clk;
    sc_signal<bool> srstn;

    sc_signal<bool> s_valid;
    sc_signal<bool> s_ready;
    sc_signal<int> s_ray_id;
    sc_signal<bool> s_hit;
    sc_signal<int> s_hit_trig_idx;
    sc_signal<float> s_t;
    sc_signal<float> s_u;
    sc_signal<float> s_v;

    sc_signal<int> pixel_idx;

    // vcd file
    sc_trace_file *tf;

    SC_HAS_PROCESS(TESTBENCH);
    TESTBENCH(const sc_module_name &mn, Bvh *bvh)
        : sc_module(mn), rtcore("rtcore", bvh), bvh(bvh), clk("clk", 2, SC_PS) {
        // link RTCORE
        rtcore.s_valid(m_valid);
        rtcore.s_ready(m_ready);
        rtcore.s_origin_x(m_origin_x);
        rtcore.s_origin_y(m_origin_y);
        rtcore.s_origin_z(m_origin_z);
        rtcore.s_dir_x(m_dir_x);
        rtcore.s_dir_y(m_dir_y);
        rtcore.s_dir_z(m_dir_z);
        rtcore.s_tmax(m_tmax);
        rtcore.s_ray_id(m_ray_id);
        rtcore.clk(clk);
        rtcore.srstn(srstn);
        rtcore.m_valid(s_valid);
        rtcore.m_ready(s_ready);
        rtcore.m_ray_id(s_ray_id);
        rtcore.m_hit(s_hit);
        rtcore.m_hit_trig_idx(s_hit_trig_idx);
        rtcore.m_t(s_t);
        rtcore.m_u(s_u);
        rtcore.m_v(s_v);

        SC_THREAD(main)

        SC_METHOD(raygen)
        sensitive << clk.posedge_event();
        dont_initialize();

        SC_METHOD(shader)
        sensitive << clk.posedge_event();
        dont_initialize();

        tf = sc_create_vcd_trace_file("wave");

        /*
        sc_trace(tf, clk, "clk");
        sc_trace(tf, srstn, "srstn");
        sc_trace(tf, m_valid, "m_valid");
        sc_trace(tf, rtcore.rd.s_alloc_ready, "rtcore.rd.s_alloc_ready");
        sc_trace(tf, rtcore.rd.s_alloc_ray_id, "rtcore.rd.s_alloc_ray_id");
        sc_trace(tf, rtcore.rd.m_valid, "rtcore.rd.m_valid");
        sc_trace(tf, rtcore.rd.m_ray_id, "rtcore.rd.m_ray_id");
        sc_trace(tf, rtcore.trv_fifo.m_valid, "rtcore.trv_fifo.m_valid");
        sc_trace(tf, rtcore.trv_fifo.m_ray_id, "rtcore.trv_fifo.m_ray_id");
        sc_trace(tf, rtcore.trv_arbitrator.s_tf_ready, "rtcore.trv_arbitrator.s_tf_ready");
        sc_trace(tf, rtcore.trv_arbitrator.s_rd_ready, "rtcore.trv_arbitrator.s_rd_ready");
        sc_trace(tf, rtcore.trv_arbitrator.m_valid, "rtcore.trv_arbitrator.m_valid");
        sc_trace(tf, rtcore.trv_arbitrator.m_ray_id, "rtcore.trv_arbitrator.m_ray_id");
        sc_trace(tf, rtcore.trv.s_ready, "rtcore.trv.s_ready");
        sc_trace(tf, rtcore.trv.m_ray_id, "rtcore.trv.m_ray_id");
        sc_trace(tf, rtcore.trv.m_ist_valid, "rtcore.trv.m_ist_valid");
        sc_trace(tf, rtcore.trv.m_ist_trig_idx, "rtcore.trv.m_ist_trig_idx");
        sc_trace(tf, rtcore.trv.m_is_last_trig, "rtcore.trv.m_is_last_trig");
        sc_trace(tf, rtcore.trv.m_pf_valid, "rtcore.trv.m_pf_valid");
        sc_trace(tf, rtcore.ist.m_valid, "rtcore.ist.m_valid");
        sc_trace(tf, rtcore.ist.m_ray_id, "rtcore.ist.m_ray_id");
        sc_trace(tf, rtcore.post_fifo.s_ready, "rtcore.post_fifo.s_ready");
        sc_trace(tf, rtcore.post_fifo.m_valid, "rtcore.post_fifo.m_valid");
        sc_trace(tf, rtcore.post_fifo.m_ray_id, "rtcore.post_fifo.m_ray_id");
        sc_trace(tf, rtcore.post.s_ready, "rtcore.post.s_ready");
        sc_trace(tf, rtcore.post.m_valid, "rtcore.post.m_valid");
        sc_trace(tf, rtcore.post.m_ray_id, "rtcore.post.m_ray_id");
        sc_trace(tf, s_ready, "s_ready");
         */
    }

    void main() {
        srstn = false;
        wait(9, SC_PS);
        srstn = true;
    }

    void raygen() {
        if (!srstn) {
            pixel_idx = 0;
            m_valid = false;
        } else {
            if (pixel_idx < width * height) m_valid = true;
            else m_valid = false;

            float j = pixel_idx % width;
            float i = pixel_idx / width;
            float dir_x = (-0.1f + horizontal * j / width) - origin_x;
            float dir_y = (0.2f - vertical * i / height) - origin_y;
            float dir_z = -1.f;

            m_origin_x = origin_x;
            m_origin_y = origin_y;
            m_origin_z = origin_z;
            m_dir_x = dir_x;
            m_dir_y = dir_y;
            m_dir_z = dir_z;
            m_tmax = FLT_MAX;

            if (m_valid && m_ready) {
                ray_id_to_pixel_idx[m_ray_id] = pixel_idx;
                pixel_idx = pixel_idx + 1;
            }
        }
    }

    void shader() {
        if (!srstn) {
            s_ready = false;
        } else {
            s_ready = true;
            if (s_valid && s_ready) {
                if (s_hit) {
                    float r = bvh->triangles[s_hit_trig_idx].n.x;
                    float g = bvh->triangles[s_hit_trig_idx].n.y;
                    float b = bvh->triangles[s_hit_trig_idx].n.z;
                    float length = sqrtf(r * r + g * g + b * b);
                    r = (r / length + 1.f) / 2.f;
                    g = (g / length + 1.f) / 2.f;
                    b = (b / length + 1.f) / 2.f;
                    framebuffer_r[ray_id_to_pixel_idx[s_ray_id]] = std::clamp(int(256.f * r), 0, 255);
                    framebuffer_g[ray_id_to_pixel_idx[s_ray_id]] = std::clamp(int(256.f * g), 0, 255);
                    framebuffer_b[ray_id_to_pixel_idx[s_ray_id]] = std::clamp(int(256.f * b), 0, 255);
                } else {
                    framebuffer_r[ray_id_to_pixel_idx[s_ray_id]] = 0;
                    framebuffer_g[ray_id_to_pixel_idx[s_ray_id]] = 0;
                    framebuffer_b[ray_id_to_pixel_idx[s_ray_id]] = 0;
                }
            }
        }
    }

    ~TESTBENCH() {
        sc_close_vcd_trace_file(tf);

        std::ofstream image_file("image.ppm");
        image_file << "P3\n" << width << ' ' << height << "\n255\n";
        for(int i = 0; i < height * width; i++) {
            image_file << framebuffer_r[i] << ' ' << framebuffer_g[i] << ' ' << framebuffer_b[i] << "\n";
        }
        image_file.close();
    }
};

#endif //RTCORE_SYSTEMC_TESTBENCH_HPP
