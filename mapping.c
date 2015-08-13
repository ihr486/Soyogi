#include "decoder.h"

int mapping_num = 0;
mapping_header_t *mapping_list = NULL;

void setup_mappings(void)
{
    mapping_num = read_unsigned_value(6) + 1;
    mapping_list = setup_ref(setup_allocate_natural(sizeof(mapping_header_t) * mapping_num));

    for(int i = 0; i < mapping_num; i++) {
        mapping_header_t *mapping = &mapping_list[i];

        uint16_t mapping_type = read_unsigned_value(16);

        if(mapping_type)
            ERROR(ERROR_MAPPING, "Unsupported mapping type: %d.\n", mapping_type);

        INFO("Mapping #%d:\n", i);

        if(read_unsigned_value(1)) {
            mapping->submaps = read_unsigned_value(4);
        } else {
            mapping->submaps = 1;
        }

        INFO("\t%d submaps.\n", mapping->submaps);

        if(read_unsigned_value(1)) {
            mapping->coupling_steps = read_unsigned_value(8) + 1;
            mapping->coupling_step_list = setup_allocate_natural(sizeof(coupling_step_t) * mapping->coupling_steps);

            coupling_step_t *step_list = setup_ref(mapping->coupling_step_list);
            for(int j = 0; j < mapping->coupling_steps; j++) {
                step_list[j].magnitude = read_unsigned_value(ilog(audio_channels - 1));
                step_list[j].angle = read_unsigned_value(ilog(audio_channels - 1));

                INFO("\tCoupling step #%d: Ch[%d] as magnitude and Ch[%d] as angle.\n", j, step_list[j].magnitude, step_list[j].angle);
            }
        } else {
            mapping->coupling_steps = 0;
        }

        if(read_unsigned_value(2))
            ERROR(ERROR_MAPPING, "Unexpected nonzero value in a reserved field.\n");

        if(mapping->submaps > 1) {
            mapping->channels = audio_channels;
            mapping->channel_list = setup_allocate_natural(sizeof(channel_t) * mapping->channels);

            channel_t *channel_list = setup_ref(mapping->channel_list);
            for(int j = 0; j < audio_channels; j++) {
                channel_list[j].mux = read_unsigned_value(4);
            }
        }

        mapping->submap_list = setup_allocate_natural(sizeof(submap_t) * mapping->submaps);

        submap_t *submap_list = setup_ref(mapping->submap_list);

        for(int j = 0; j < mapping->submaps; j++) {
            read_unsigned_value(8);

            submap_list[j].floor = read_unsigned_value(8);
            submap_list[j].residue = read_unsigned_value(8);

            INFO("\tSubmap #%d: Floor %d, Residue %d.\n", j, submap_list[j].floor, submap_list[j].residue);
        }
    }
}
