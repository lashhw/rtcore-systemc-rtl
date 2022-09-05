#ifndef RTCORE_SYSTEMC_IST_HPP
#define RTCORE_SYSTEMC_IST_HPP

SC_MODULE(IST) {
    // ports
    sc_in<bool> s_valid;
    sc_in<int> s_ray_id;
    sc_in<int> s_ist_trig_idx;
    sc_in<bool> s_is_last_trig;

    sc_in<bool> clk;
    sc_in<bool> srstn;

    sc_out<bool> m_valid;
    sc_out<int> m_ray_id;

    // high-level objects
    Bvh *bvh;
    RayState *ray_states;

    SC_HAS_PROCESS(IST);
    IST(sc_module_name mn, Bvh *bvh, RayState *ray_states)
        : sc_module(mn), bvh(bvh), ray_states(ray_states) {
        SC_METHOD(main)
        sensitive << clk.pos();
        dont_initialize();

        SC_METHOD(update_m_valid)
        sensitive << s_is_last_trig;

        SC_METHOD(update_m_ray_id)
        sensitive << s_ray_id;
    }

    void main() {
        // load triangle from memory
        Triangle* trig = &bvh->triangles[s_ist_trig_idx];
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

        // load ray data from memory
        float origin_x = ray_states[s_ray_id].origin_x;
        float origin_y = ray_states[s_ray_id].origin_y;
        float origin_z = ray_states[s_ray_id].origin_z;
        float dir_x = ray_states[s_ray_id].dir_x;
        float dir_y = ray_states[s_ray_id].dir_y;
        float dir_z = ray_states[s_ray_id].dir_z;
        float tmax = ray_states[s_ray_id].tmax;

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
           ray_states[s_ray_id].tmax = t_tmp;
           ray_states[s_ray_id].hit = true;
           ray_states[s_ray_id].hit_trig_idx = s_ist_trig_idx;
           ray_states[s_ray_id].u = u_tmp;
           ray_states[s_ray_id].v = v_tmp;
        }
    }

    void update_m_valid() {
        m_valid = s_valid && s_is_last_trig;
    }

    void update_m_ray_id() {
        m_ray_id = s_ray_id;
    }
};

#endif //RTCORE_SYSTEMC_IST_HPP
