grep "#define" radlib.h | grep -v "_RADLIB_H_" | sed -E 's/#define. *RAD_([A-Z_0-9]+)[^0-9]+([0-9]+)/REGISTER_LONG_CONSTANT("RADIUS_\1", \2, CONST_PERSISTENT);/' > radius_init_const.h
grep "#define" radlib_vs.h | grep -v "_RADLIB_VS_H_" | sed -E 's/#define. *RAD_([A-Z_0-9]+)[^0-9]+([0-9]+)/REGISTER_LONG_CONSTANT("RADIUS_\1", \2, CONST_PERSISTENT);/'  >> radius_init_const.h
