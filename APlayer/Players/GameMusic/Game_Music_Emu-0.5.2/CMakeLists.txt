cmake_minimum_required(VERSION 2.6)
project(GME)

add_library(gme gme/gme.cpp gme/Gme_File.cpp gme/Music_Emu.cpp
	gme/Classic_Emu.cpp gme/Multi_Buffer.cpp gme/Blip_Buffer.cpp
	gme/Dual_Resampler.cpp gme/Fir_Resampler.cpp gme/Effects_Buffer.cpp
	
	gme/Data_Reader.cpp
	
	gme/Ay_Emu.cpp gme/Ay_Cpu.cpp gme/Ay_Apu.cpp
	
	gme/Sap_Emu.cpp gme/Sap_Apu.cpp gme/Sap_Cpu.cpp 
	
	gme/Nsf_Emu.cpp gme/Nes_Fme7_Apu.cpp gme/Nes_Namco_Apu.cpp gme/Nes_Cpu.cpp
	gme/Nes_Vrc6_Apu.cpp gme/Nes_Apu.cpp gme/Nes_Oscs.cpp
	
	gme/Gbs_Emu.cpp gme/Gb_Cpu.cpp gme/Gb_Apu.cpp gme/Gb_Oscs.cpp
	
	gme/Gym_Emu.cpp
	
	gme/Hes_Emu.cpp gme/Hes_Cpu.cpp gme/Hes_Apu.cpp
	
	gme/Kss_Emu.cpp gme/Kss_Scc_Apu.cpp gme/Kss_Cpu.cpp
	
	gme/Nsfe_Emu.cpp
	
	gme/Spc_Emu.cpp gme/Snes_Spc.cpp gme/Spc_Cpu.cpp gme/Spc_Dsp.cpp
	
	gme/Vgm_Emu.cpp gme/Ym2612_Emu.cpp gme/Vgm_Emu_Impl.cpp gme/Sms_Apu.cpp
	gme/Ym2413_Emu.cpp
)

include_directories(.)
add_executable(basics demo/basics.c demo/Wave_Writer.cpp)
target_link_libraries(basics gme)
