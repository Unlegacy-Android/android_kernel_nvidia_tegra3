ifeq ($(CONFIG_ARCH_TEGRA_2x_SOC),y)
zreladdr-y	:= 0x00008000
params_phys-y	:= 0x00000100
initrd_phys-y	:= 0x00800000
else
zreladdr-y	:= 0x80008000
params_phys-y	:= 0x80000100
initrd_phys-y	:= 0x80800000
endif

dtb-$(CONFIG_MACH_HARMONY) += tegra-harmony.dtb
dtb-$(CONFIG_MACH_PAZ00) += tegra-paz00.dtb
dtb-$(CONFIG_MACH_SEABOARD) += tegra-seaboard.dtb
dtb-$(CONFIG_MACH_TRIMSLICE) += tegra-trimslice.dtb
dtb-$(CONFIG_MACH_VENTANA) += tegra-ventana.dtb
dtb-$(CONFIG_MACH_CARDHU) += tegra-cardhu.dtb
