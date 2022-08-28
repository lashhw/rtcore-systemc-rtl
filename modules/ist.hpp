#ifndef DEFAULT_SYSTEMC_IST_HPP
#define DEFAULT_SYSTEMC_IST_HPP

SC_MODULE(IST) {
    // ports
    sc_in<bool> clk;
    sc_in<bool> reset;
    sc_in<Ray> ray;
    sc_in<int> trig_idx;
    sc_out<bool> isected;
    sc_out<float> t;
    sc_out<float> u;
    sc_out<float> v;

    // internal states
    Bvh *bvh;

    SC_HAS_PROCESS(IST);
    IST(sc_module_name mn, Bvh *bvh) : sc_module(mn), bvh(bvh) {
        SC_METHOD(main);
        sensitive << clk.pos();
        dont_initialize();
    }

    void main() {
        Triangle *trig = &bvh->triangles[trig_idx];

        Vec3 c = trig->p0 - ray.read().origin;
        Vec3 r = cross(ray.read().unit_d, c);
        float inv_det = 1.f / dot(ray.read().unit_d, trig->n);

        float tmp_u, tmp_v;
        tmp_u = inv_det * dot(trig->e2, r);
        tmp_v = inv_det * dot(trig->e1, r);

        bool tmp_isected = false;
        if (tmp_u >= 0.0f && tmp_v >= 0.0f && (tmp_u + tmp_v) <= 1.0f) {
            float tmp_t = inv_det * dot(c, trig->n);
            if (0 < tmp_t && tmp_t <= ray.read().tmax) {
                tmp_isected = true;
                t = tmp_t;
                u = tmp_u;
                v = tmp_v;
            }
        }

        isected = tmp_isected;
    }
};

#endif //DEFAULT_SYSTEMC_IST_HPP
