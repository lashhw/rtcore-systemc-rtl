#ifndef DEFAULT_SYSTEMC_TESTBENCH_HPP
#define DEFAULT_SYSTEMC_TESTBENCH_HPP

#include <fstream>
#include "ist.hpp"
#include "trv.hpp"

SC_MODULE(Testbench) {
    // signals
    sc_clock clk;
    sc_signal<bool> reset;
    sc_signal<bool> start;
    sc_signal<float> origin_x;
    sc_signal<float> origin_y;
    sc_signal<float> origin_z;
    sc_signal<float> dir_x;
    sc_signal<float> dir_y;
    sc_signal<float> dir_z;
    sc_signal<float> tmax;
    sc_signal<bool> done;
    sc_signal<bool> isected;
    sc_signal<float> t;
    sc_signal<float> u;
    sc_signal<float> v;
    sc_signal<int> trig_idx;

    // submodules
    TRV trv;

    // internal states
    Bvh *bvh;

    // vcd file
    sc_trace_file* tf;

    SC_HAS_PROCESS(Testbench);
    Testbench(sc_module_name mn, Bvh *bvh)
        : sc_module(mn), clk("clk", 2, SC_PS), trv("trv", bvh), bvh(bvh) {
        // link TRV
        trv.clk(clk);
        trv.reset(reset);
        trv.start(start);
        trv.origin_x(origin_x);
        trv.origin_y(origin_y);
        trv.origin_z(origin_z);
        trv.dir_x(dir_x);
        trv.dir_y(dir_y);
        trv.dir_z(dir_z);
        trv.tmax(tmax);
        trv.done(done);
        trv.isected(isected);
        trv.t(t);
        trv.u(u);
        trv.v(v);
        trv.trig_idx(trig_idx);

        SC_THREAD(main);

        /*
        tf = sc_create_vcd_trace_file("wave");
        sc_trace(tf, clk, "clk");
        sc_trace(tf, reset, "reset");
        sc_trace(tf, start, "start");
        sc_trace(tf, origin_x, "origin_x");
        sc_trace(tf, origin_y, "origin_y");
        sc_trace(tf, origin_z, "origin_z");
        sc_trace(tf, dir_x, "dir_x");
        sc_trace(tf, dir_y, "dir_y");
        sc_trace(tf, dir_z, "dir_z");
        sc_trace(tf, tmax, "tmax");
        sc_trace(tf, done, "done");
        sc_trace(tf, isected, "isected");
        sc_trace(tf, t, "t");
        sc_trace(tf, u, "u");
        sc_trace(tf, v, "v");
        sc_trace(tf, trig_idx, "trig_idx");

        sc_trace(tf, trv.state, "trv.state");
        sc_trace(tf, trv.entry_left, "trv.entry_left");
        sc_trace(tf, trv.entry_right, "trv.entry_right");
        sc_trace(tf, trv.curr_node_idx, "trv.curr_node_idx");
        sc_trace(tf, trv.left_node_idx, "trv.left_node_idx");
        sc_trace(tf, trv.octant_x, "trv.octant_x");
        sc_trace(tf, trv.octant_y, "trv.octant_y");
        sc_trace(tf, trv.octant_z, "trv.octant_z");
        sc_trace(tf, trv.inv_dir_x, "trv.inv_dir_x");
        sc_trace(tf, trv.inv_dir_y, "trv.inv_dir_y");
        sc_trace(tf, trv.inv_dir_z, "trv.inv_dir_z");
        sc_trace(tf, trv.scaled_origin_x, "trv.scaled_origin_x");
        sc_trace(tf, trv.scaled_origin_y, "trv.scaled_origin_y");
        sc_trace(tf, trv.scaled_origin_z, "trv.scaled_origin_z");

        sc_trace(tf, trv.ist.isected, "trv.ist.isected");
        sc_trace(tf, trv.ist.t, "trv.ist.t");
        sc_trace(tf, trv.ist.u, "trv.ist.u");
        sc_trace(tf, trv.ist.v, "trv.ist.v");
         */
    }

    ~Testbench() {
        // sc_close_vcd_trace_file(tf);
    }

    void main() {
        reset = false;
        start = false;
        wait(5, SC_PS);
        reset = true;

        origin_x = 0.f;
        origin_y = 0.1f;
        origin_z = 1.f;

        float horizontal = 0.2f;
        float vertical = 0.2f;
        int width = 100;
        int height = 100;

        std::ofstream file("image.ppm");
        file << "P3\n" << width << ' ' << height << "\n255\n";
        for(int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                dir_x = (-0.1f + horizontal * j / width) - origin_x;
                dir_y = (0.2f - vertical * i / height) - origin_y;
                dir_z = -1.f;
                tmax = FLT_MAX;
                wait(2, SC_PS);
                start.write(true);
                wait(2, SC_PS);
                start = false;
                wait(done.posedge_event());
                if (isected) {
                    float r = bvh->triangles[trig_idx].n.x;
                    float g = bvh->triangles[trig_idx].n.y;
                    float b = bvh->triangles[trig_idx].n.z;
                    float length = sqrtf(r * r + g * g + b * b);
                    r = (r / length + 1.f) / 2.f;
                    g = (g / length + 1.f) / 2.f;
                    b = (b / length + 1.f) / 2.f;
                    int ir = std::clamp(int(256.f * r), 0, 255);
                    int ig = std::clamp(int(256.f * g), 0, 255);
                    int ib = std::clamp(int(256.f * b), 0, 255);
                    file << ir << ' ' << ig << ' ' << ib << "\n";
                } else {
                    file << "0 0 0\n";
                }
            }
        }
    }
};


#endif //DEFAULT_SYSTEMC_TESTBENCH_HPP
