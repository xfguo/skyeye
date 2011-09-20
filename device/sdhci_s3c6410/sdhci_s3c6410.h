#ifndef __SDHCI_S3C6410_H__
#define __SDHCI_S3C6410_H__
typedef struct sdhci_reg{
}sdhci_reg_t;

typedef struct s3c6410_sdhci_device{
	conf_object_t* obj;
	sdhci_reg_t* regs;
}s3c6410_sdhci_device;
#endif
