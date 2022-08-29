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
        // load triangle from memory
        Triangle* trig = &bvh->triangles[trig_idx];
        float n_x = trig->n.x;
        float n_y = trig->n.y;
        float n_z = trig->n.z;
        float p0_x = trig->p0.x;
        float p0_y = trig->p0.y;
        float p0_z = trig->p0.z;
        float e1_x = trig->e1.x;
        float e1_y = trig->e1.y;
        float e1_z = trig->e1.z;
        float e2_x = trig->e2.x;
        float e2_y = trig->e2.y;
        float e2_z = trig->e2.z;

        float c_x = p0_x - origin_x;
        float c_y = p0_y - origin_y;
        float c_z = p0_z - origin_z;
        float r_x = dir_y * c_z - dir_z * c_y;
        float r_y = dir_z * c_x - dir_x * c_z;
        float r_z = dir_x * c_y - dir_y * c_x;
        float inv_det = 1.f / (dir_x * n_x + dir_y * n_y + dir_z * n_z);

        float u_tmp, v_tmp, t_tmp;
        u_tmp = inv_det * (e2_x * r_x + e2_y * r_y + e2_z * r_z);
        v_tmp = inv_det * (e1_x * r_x + e1_y * r_y + e1_z * r_z);
        t_tmp = inv_det * (c_x * n_x + c_y * n_y + c_z * n_z);

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
