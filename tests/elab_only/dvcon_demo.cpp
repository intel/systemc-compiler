//
// Created by ripopov on 6/14/18.
//
#include <systemc.h>

//
// Shown coding style is very advanced. We are not advocating for advanced C++
// for HW design. The goal of this example is to demonstrate potential language
// constructs elaboration tool may encounter.
//


template <typename AddrT>
struct address_range {
    AddrT start_addr;
    AddrT end_addr;
};

template <typename AddrT>
struct address_decoder : sc_module {

    sc_in <AddrT>            address{"address"};
    sc_vector<sc_out<bool>>  slave_select{"slave_select"};

    address_decoder(sc_module_name, std::vector <address_range<AddrT>> addr_map)
        : address_map(std::move(addr_map))
    {
        slave_select.init(address_map.size());

        SC_HAS_PROCESS(address_decoder);
        SC_METHOD(slave_select_method);
        sensitive << address;
    }

    const std::vector <address_range<AddrT>>  address_map;

    void slave_select_method() {
        for (size_t i = 0; i < address_map.size(); ++i) {
            slave_select[i] = false;
            if (address >= address_map[i].start_addr &&
                address <= address_map[i].end_addr)
                slave_select[i] = true;
        }
    }
};

struct apb_i2c : sc_module {
    sc_in <bool>   en{"en"};
    apb_i2c(sc_module_name) {
        SC_HAS_PROCESS(apb_i2c);
        SC_METHOD(device_method);
        sensitive << en;
    }

    void device_method() {
        cout << name();
        if (en)
            cout << " enabled\n";
        else
            cout << " disabled\n";
    }
};

struct apb_uart : apb_i2c {
    apb_uart(sc_module_name nm) : apb_i2c(nm) {}
};

template <typename AddrT>
struct test_system : sc_module {

    sc_clock                    clk{"clk", 10, SC_NS};
    sc_signal<AddrT>            address{"address"};
    std::vector<sc_module*>     devices;
    sc_vector<sc_signal<bool>>  slave_select{"slave_select"};
    address_decoder<AddrT>      *decoder;

    std::vector <address_range<AddrT>>  address_map;

    test_system (sc_module_name, std::string config_file) {
        ifstream ifile(config_file);
        ifile >> hex;
        unsigned N_DEVICES;
        ifile >> N_DEVICES;

        slave_select.init(N_DEVICES);

        for (unsigned dev_id = 0; dev_id < N_DEVICES; ++dev_id) {
            unsigned start_addr, end_addr;
            std::string device_type, device_name;

            ifile >> start_addr >> end_addr >> device_type >> device_name;

            if (device_type == "apb_uart") {
                apb_uart * adev = new apb_uart(device_name.c_str());
                adev->en(slave_select[dev_id]);
                devices.push_back(adev);
            }
            else {
                apb_i2c * bdev = new apb_i2c(device_name.c_str());
                bdev->en(slave_select[dev_id]);
                devices.push_back(bdev);
            }

            address_map.push_back({start_addr, end_addr});
        }

        decoder = new address_decoder<AddrT>("decoder", address_map);
        decoder->address(address);
        for (size_t dev_id = 0; dev_id < N_DEVICES ; ++dev_id)
            decoder->slave_select[dev_id](slave_select[dev_id]);

        SC_HAS_PROCESS(test_system);
        SC_THREAD(test_thread);
        sensitive << clk.posedge_event();
    }

    void test_thread() {
        for (size_t dev_id = 0; dev_id < address_map.size(); ++dev_id) {
            address = address_map[dev_id].start_addr;
            wait();
        }
        sc_stop();
    }

};

std::string file_dir() {
    std::string filename(__FILE__);
    size_t found = filename.find_last_of("/");
    return filename.substr(0 , found);
}

int sc_main (int argc, char **argv) {

    std::string input_file = file_dir() + "/dvcon_demo.txt";
    cout << "Using config file: "<< input_file << endl;

    test_system<unsigned>  sys_inst("sys_inst", input_file);
    sc_start();

    return 0;
}
