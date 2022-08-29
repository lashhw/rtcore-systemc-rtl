#ifndef DEFAULT_SYSTEMC_IST_HPP
#define DEFAULT_SYSTEMC_IST_HPP

SC_MODULE(IST) {
    // ports
    sc_in<bool> clk;
    sc_in<bool> reset;
    sc_in<int> trig_idx;
    sc_in<float> origin_x;
    sc_in<float> origin_y;
    sc_in<float> origin_z;
    sc_in<float> dir_x;
    sc_in<float> dir_y;
    sc_in<float> dir_z;
    sc_in<float> tmax;
    sc_out<bool> isected;
    sc_out<float> t;
    sc_out<float> u;
    sc_out<float> v;

    // internal states
    Bvh *bvh;

    SC_HAS_PROCESS(IST);
    IST(sc_module_name mn, Bvh *bvh) : sc_module(mn), bvh(bvh) {
        SC_METHOD(main)
        sensitive << trig_idx << origin_x << origin_y << origin_z << dir_x << dir_y << dir_z << tmax;
    }

    void main() {
        Triangle *trig = &bvh->triangles[trig_idx];

        Vec3 c = trig->p0 - Vec3(origin_x, origin_y, origin_z);
        Vec3 r = cross(Vec3(dir_x, dir_y, dir_z), c);
        float inv_det = 1.f / dot(Vec3(dir_x, dir_y, dir_z), trig->n);

        float u_tmp, v_tmp, t_tmp;
        u_tmp = inv_det * dot(trig->e2, r);
        v_tmp = inv_det * dot(trig->e1, r);
        t_tmp = inv_det * dot(c, trig->n);

        if (u_tmp >= 0.0f && v_tmp >= 0.0f && (u_tmp + v_tmp) <= 1.0f && 0 < t_tmp && t_tmp <= tmax) {
            isected = true;
            t = t_tmp;
            u = u_tmp;
            v = v_tmp;
        } else {
            isected = false;
        }
    }
};

#endif //DEFAULT_SYSTEMC_IST_HPP
