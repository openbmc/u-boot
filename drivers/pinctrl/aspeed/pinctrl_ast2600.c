// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <asm/arch/pinctrl.h>
#include <asm/arch/scu_ast2600.h>
#include <dm/pinctrl.h>
#include "pinctrl-aspeed.h"
/*
 * This driver works with very simple configuration that has the same name
 * for group and function. This way it is compatible with the Linux Kernel
 * driver.
 */

struct ast2600_pinctrl_priv {
	struct ast2600_scu *scu;
};

static int ast2600_pinctrl_probe(struct udevice *dev)
{
	struct ast2600_pinctrl_priv *priv = dev_get_priv(dev);
	struct udevice *clk_dev;	
	int ret = 0;

	/* find SCU base address from clock device */
	ret = uclass_get_device_by_driver(UCLASS_CLK, DM_GET_DRIVER(aspeed_scu),
                                          &clk_dev);
    if (ret) {
            debug("clock device not found\n");
            return ret;
    }

	priv->scu = devfdt_get_addr_ptr(clk_dev);
	if (IS_ERR(priv->scu)) {
	        debug("%s(): can't get SCU\n", __func__);
	        return PTR_ERR(priv->scu);
	}

	return 0;
}

static struct aspeed_sig_desc mac1_link[] = {
#ifdef CONFIG_FPGA_ASPEED
	{ 0x410, BIT(4), 0 },
#else
	{ 0x400, GENMASK(11, 0), 0 },
	{ 0x410, BIT(4), 0 },
	{ 0x470, BIT(4), 1 },
#endif
};

static struct aspeed_sig_desc mac2_link[] = {
	{ 0x400, GENMASK(23, 12), 0 },
	{ 0x410, BIT(5), 0	},
	{ 0x470, BIT(5), 1 },
};

static struct aspeed_sig_desc mac3_link[] = {	
	{ 0x410, GENMASK(27, 16), 0	},
	{ 0x410, BIT(6), 0		},
	{ 0x470, BIT(6), 1      	},
};

static struct aspeed_sig_desc mac4_link[] = {
	{ 0x410, GENMASK(31, 28), 1	},
	{ 0x4b0, GENMASK(31, 28), 0	},
	{ 0x474, GENMASK(7, 0), 1	},
	{ 0x414, GENMASK(7, 0), 1	},
	{ 0x4b4, GENMASK(7, 0), 0	},
	{ 0x410, BIT(7), 0		},
	{ 0x470, BIT(7), 1		},
};

static struct aspeed_sig_desc mdio1_link[] = {
	{ 0x430, BIT(17) | BIT(16), 0	},
};

static struct aspeed_sig_desc mdio2_link[] = {
	{ 0x470, BIT(13) | BIT(12), 1	},
	{ 0x410, BIT(13) | BIT(12), 0	},
};

static struct aspeed_sig_desc mdio3_link[] = {
	{ 0x470, BIT(1) | BIT(0), 1	},
	{ 0x410, BIT(1) | BIT(0), 0	},
};

static struct aspeed_sig_desc mdio4_link[] = {
	{ 0x470, BIT(3) | BIT(2), 1	},
	{ 0x410, BIT(3) | BIT(2), 0	},
};

static struct aspeed_sig_desc sdio2_link[] = {
	{ 0x414, GENMASK(23, 16), 1	},
	{ 0x4B4, GENMASK(23, 16), 0	},
	{ 0x450, BIT(1), 0		},
};

static struct aspeed_sig_desc sdio1_link[] = {
	{ 0x414, GENMASK(15, 8), 0	},
};	

//when sdio1 8bits, sdio2 can't use
static struct aspeed_sig_desc sdio1_8bit_link[] = {
	{ 0x414, GENMASK(15, 8), 0	},
	{ 0x4b4, GENMASK(21, 18), 0	},
	{ 0x450, BIT(3), 0	},
	{ 0x450, BIT(1), 1	},
};	

static struct aspeed_sig_desc emmc_link[] = {
	{ 0x400, GENMASK(31, 24), 0 },
#if 0	//8bit emmc	
	{ 0x404, GENMASK(3, 0), 0 },
	{ 0x500, BIT(3), 1 },
	{ 0x500, BIT(5), 1 },
#endif	
};

static const struct aspeed_group_config ast2600_groups[] = {
	{ "MAC1LINK", ARRAY_SIZE(mac1_link), mac1_link },
	{ "MAC2LINK", ARRAY_SIZE(mac2_link), mac2_link },
	{ "MAC3LINK", ARRAY_SIZE(mac3_link), mac3_link },
	{ "MAC4LINK", ARRAY_SIZE(mac4_link), mac4_link },
	{ "MDIO1", ARRAY_SIZE(mdio1_link), mdio1_link },
	{ "MDIO2", ARRAY_SIZE(mdio2_link), mdio2_link },
	{ "MDIO3", ARRAY_SIZE(mdio3_link), mdio3_link },
	{ "MDIO4", ARRAY_SIZE(mdio4_link), mdio4_link },
	{ "SD1", ARRAY_SIZE(sdio1_link), sdio1_link },
	{ "SD1_8bits", ARRAY_SIZE(sdio1_8bit_link), sdio1_8bit_link },
	{ "SD2", ARRAY_SIZE(sdio2_link), sdio2_link },
	{ "EMMC", ARRAY_SIZE(emmc_link), emmc_link },
};

static int ast2600_pinctrl_get_groups_count(struct udevice *dev)
{
	debug("PINCTRL: get_(functions/groups)_count\n");

	return ARRAY_SIZE(ast2600_groups);
}

static const char *ast2600_pinctrl_get_group_name(struct udevice *dev,
						  unsigned selector)
{
	debug("PINCTRL: get_(function/group)_name %u\n", selector);

	return ast2600_groups[selector].group_name;
}

static int ast2600_pinctrl_group_set(struct udevice *dev, unsigned selector,
				     unsigned func_selector)
{
	struct ast2600_pinctrl_priv *priv = dev_get_priv(dev);
	const struct aspeed_group_config *config;
	const struct aspeed_sig_desc *descs;
	u32 ctrl_reg = (u32)priv->scu;
	u32 i;

	debug("PINCTRL: group_set <%u, %u>\n", selector, func_selector);
	if (selector >= ARRAY_SIZE(ast2600_groups))
		return -EINVAL;

	config = &ast2600_groups[selector];
	for (i = 0; i < config->ndescs; i++) {
		descs = &config->descs[i];
		if (descs->clr) {
			clrbits_le32((u32)ctrl_reg + descs->offset,
				     descs->reg_set);
		} else {
			setbits_le32((u32)ctrl_reg + descs->offset,
				     descs->reg_set);
		}
	}

	return 0;
}

static struct pinctrl_ops ast2600_pinctrl_ops = {
	.set_state = pinctrl_generic_set_state,
	.get_groups_count = ast2600_pinctrl_get_groups_count,
	.get_group_name = ast2600_pinctrl_get_group_name,
	.get_functions_count = ast2600_pinctrl_get_groups_count,
	.get_function_name = ast2600_pinctrl_get_group_name,
	.pinmux_group_set = ast2600_pinctrl_group_set,
};

static const struct udevice_id ast2600_pinctrl_ids[] = {
	{ .compatible = "aspeed,g6-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_aspeed) = {
	.name = "aspeed_ast2600_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = ast2600_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct ast2600_pinctrl_priv),
	.ops = &ast2600_pinctrl_ops,
	.probe = ast2600_pinctrl_probe,
};