#include "decoder.h"

int mode_num = 0;
vorbis_mode_t *mode_list = NULL;

void setup_modes(void)
{
    mode_num = read_unsigned_value(6) + 1;
    mode_list = setup_ref(setup_allocate_natural(sizeof(vorbis_mode_t) * mode_num));

    for(int i = 0; i < mode_num; i++) {
        mode_list[i].blockflag = read_unsigned_value(1);
        
        uint16_t windowtype = read_unsigned_value(16);
        if(windowtype)
            ERROR(ERROR_MODE, "Unsupported window type: %d.\n", windowtype);

        uint16_t transformtype = read_unsigned_value(16);
        if(transformtype)
            ERROR(ERROR_MODE, "Unsupported transform type: %d.\n", transformtype);

        mode_list[i].mapping = read_unsigned_value(8);

        INFO("Mode #%d: Block %d, mapping %d.\n", i, mode_list[i].blockflag, mode_list[i].mapping);
    }

    if(!read_unsigned_value(1))
        ERROR(ERROR_SETUP, "Framing error at the end of setup packet.\n");
}
