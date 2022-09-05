#ifndef RTCORE_SYSTEMC_TRV_ARBITRATOR_HPP
#define RTCORE_SYSTEMC_TRV_ARBITRATOR_HPP

SC_MODULE(TRV_ARBITRATOR) {
    // ports
    sc_in<bool> s_tf_valid;
    sc_out<bool> s_tf_ready;
    sc_in<int> s_tf_ray_id;

    sc_in<bool> s_rd_valid;
    sc_out<bool> s_rd_ready;
    sc_in<int> s_rd_ray_id;

    sc_out<bool> m_valid;
    sc_in<bool> m_ready;
    sc_out<int> m_ray_id;

    SC_CTOR(TRV_ARBITRATOR) {
        SC_METHOD(main)
        sensitive << s_tf_valid << s_tf_ray_id
                  << s_rd_valid << s_rd_ray_id
                  << m_ready;
    }

    void main() {
        if (s_tf_valid) {
            s_tf_ready = true;
            s_rd_ready = false;
            m_valid = true;
            m_ray_id = s_tf_ray_id;
        } else if (s_rd_valid) {
            s_tf_ready = false;
            s_rd_ready = true;
            m_valid = true;
            m_ray_id = s_rd_ray_id;
        } else {
            s_rd_ready = false;
            s_tf_ready = false;
            m_valid = false;
        }
    }
};

#endif //RTCORE_SYSTEMC_TRV_ARBITRATOR_HPP
