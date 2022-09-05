#ifndef RTCORE_SYSTEMC_POST_HPP
#define RTCORE_SYSTEMC_POST_HPP

SC_MODULE(POST) {
    // ports
    sc_in<bool> s_valid;
    sc_out<bool> s_ready;
    sc_in<int> s_ray_id;

    sc_in<bool> clk;
    sc_in<bool> srstn;

    sc_out<bool> m_valid;
    sc_in<bool> m_ready;
    sc_out<int> m_ray_id;
    sc_out<bool> m_hit;
    sc_out<int> m_hit_trig_idx;
    sc_out<float> m_t;
    sc_out<float> m_u;
    sc_out<float> m_v;

    // high-level objects
    RayState *ray_states;

    // internal signals
    sc_signal<bool> valid;

    SC_HAS_PROCESS(POST);
    POST(const sc_module_name &mn, RayState *ray_states) : sc_module(mn), ray_states(ray_states) {
        SC_METHOD(main)
        sensitive << clk.pos();
        dont_initialize();

        SC_METHOD(update_s_ready)
        sensitive << valid;

        SC_METHOD(update_m_valid)
        sensitive << m_ready << valid;
    }

    void main() {
        if (!srstn) {
            valid = false;
        } else {
            if (s_valid && s_ready) {
                valid = true;
                m_ray_id = s_ray_id;
                m_hit = ray_states[s_ray_id].hit;
                m_hit_trig_idx = ray_states[s_ray_id].hit_trig_idx;
                m_t = ray_states[s_ray_id].tmax;
                m_u = ray_states[s_ray_id].u;
                m_v = ray_states[s_ray_id].v;
            }
            if (m_valid && m_ready) {
                valid = false;
            }
        }
    }

    void update_s_ready() {
        s_ready = !valid;
    }

    void update_m_valid() {
        m_valid = (m_ready && valid);
    }
};

#endif //RTCORE_SYSTEMC_POST_HPP
