#ifndef RTCORE_SYSTEMC_RAYGEN_HPP
#define RTCORE_SYSTEMC_RAYGEN_HPP

template<int Width, int Height>
SC_MODULE(RAYGEN) {
    // parameters
    static constexpr float origin_x = 0.f;
    static constexpr float origin_y = 0.1f;
    static constexpr float origin_z = 1.f;
    static constexpr float horizontal = 0.2f;
    static constexpr float vertical = 0.2f;

    // ports
    sc_in<bool> clk;
    sc_in<bool> srstn;

    sc_out<bool> m_valid;
    sc_in<bool> m_ready;
    sc_out<float> m_origin_x;
    sc_out<float> m_origin_y;
    sc_out<float> m_origin_z;
    sc_out<float> m_dir_x;
    sc_out<float> m_dir_y;
    sc_out<float> m_dir_z;
    sc_out<float> m_tmax;
    sc_in<int> m_ray_id;

    // internal signals
    sc_signal<int> pixel_idx;

    // high-level objects
    std::unordered_map<int, int> *ray_id_to_pixel_idx;

    SC_HAS_PROCESS(RAYGEN);
    RAYGEN(const sc_module_name &mn, std::unordered_map<int, int> *ray_id_to_pixel_idx)
        : sc_module(mn), ray_id_to_pixel_idx(ray_id_to_pixel_idx) {
        SC_METHOD(main)
        sensitive << clk.pos();
        dont_initialize();

        SC_METHOD(update_m_valid)
        sensitive << srstn << pixel_idx;

        SC_METHOD(update_m_dir)
        sensitive << pixel_idx << m_origin_x << m_origin_y;
    }

    void main() {
        if (!srstn) {
            m_origin_x = origin_x;
            m_origin_y = origin_y;
            m_origin_z = origin_z;
            m_tmax = FLT_MAX;
            pixel_idx = 0;
        } else {
            if (m_valid && m_ready) {
                (*ray_id_to_pixel_idx)[m_ray_id] = pixel_idx;
                pixel_idx = pixel_idx + 1;
            }
        }
    }

    void update_m_valid() {
        m_valid = (srstn && pixel_idx < Width * Height);
    }

    void update_m_dir() {
        float j = pixel_idx % Width;
        float i = pixel_idx / Width;
        m_dir_x = (-0.1f + horizontal * j / Width) - origin_x;
        m_dir_y = (0.2f - vertical * i / Height) - origin_y;
        m_dir_z = -1.f;
    }
};

#endif //RTCORE_SYSTEMC_RAYGEN_HPP
