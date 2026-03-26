/**
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
*
 * ESP-Drone Firmware
 *
 * Copyright 2019-2020  Espressif Systems (Shanghai)
 * Copyright (C) 2011-2012 Bitcraze AB
 * Modified by Cedarville University 2025-2026
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * configblock.c - Simple static implementation of the config block
 */
#define DEBUG_MODULE "CFGBLK"

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "config.h"
#include "debug_cf.h"
// #include "i2cdev.h"
#include "configblock.h"
// #include "eeprom.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_err.h"

#define NVS_NAMESPACE "cf_cfg"
#define NVS_KEY       "configblock"

/* Internal format of the config block */
#define MAGIC 0x43427830
#define VERSION 1
#define HEADER_SIZE_BYTES 5 // magic + version
#define OVERHEAD_SIZE_BYTES (HEADER_SIZE_BYTES + 1) // + cksum

// Old versions
struct configblock_v0_s {
  /* header */
  uint32_t magic;
  uint8_t  version;
  /* Content */
  uint8_t radioChannel;
  uint8_t radioSpeed;
  float calibPitch;
  float calibRoll;
  /* Simple modulo 256 checksum */
  uint8_t cksum;
} __attribute__((__packed__));

// Current version
struct configblock_v1_s {
  /* header */
  uint32_t magic;
  uint8_t  version;
  /* Content */
  uint8_t radioChannel;
  uint8_t radioSpeed;
  float calibPitch;
  float calibRoll;
  uint8_t radioAddress_upper;
  uint32_t radioAddress_lower;
  /* Simple modulo 256 checksum */
  uint8_t cksum;
} __attribute__((__packed__));

// Set version 1 as current version
typedef struct configblock_v1_s configblock_t;

static configblock_t configblock;
static configblock_t configblockDefault =
{
    .magic = MAGIC,
    .version = VERSION,
    .radioChannel = RADIO_CHANNEL,
    .radioSpeed = RADIO_DATARATE,
    .calibPitch = 0.0,
    .calibRoll = 0.0,
    .radioAddress_upper = ((uint64_t)RADIO_ADDRESS >> 32),
    .radioAddress_lower = (RADIO_ADDRESS & 0xFFFFFFFFULL),
};

static const uint32_t configblockSizes[] =
{
  sizeof(struct configblock_v0_s),
  sizeof(struct configblock_v1_s),
};

static bool isInit = false;
static bool cb_ok = false;

static bool configblockCheckMagic(configblock_t *configblock);
static bool configblockCheckVersion(configblock_t *configblock);
static bool configblockCheckChecksum(configblock_t *configblock);
static bool configblockCheckDataIntegrity(uint8_t *data, uint8_t version);
static bool configblockWrite(configblock_t *configblock);
static bool configblockCopyToNewVersion(configblock_t *configblockSaved, configblock_t *configblockNew);

static uint8_t calculate_cksum(void* data, size_t len)
{
  unsigned char* c = data;
  int i;
  unsigned char cksum=0;
  for (i=0; i<len; i++)
    cksum += *(c++);

  return cksum;
}

static nvs_handle_t nvs;

static void configblockResetToDefaults(configblock_t *cb)
{
    memcpy(cb, &configblockDefault, sizeof(*cb));
}

static bool configblockValidate(configblock_t *cb)
{
    return true;
}

int configblockInit(void)
{
    if(isInit)
        return 0;

    esp_err_t err;
    size_t size = sizeof(configblock);

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs);
    if (err != ESP_OK) {
        // Force defaults if NVS is unavailable
        configblockResetToDefaults(&configblock);
        return -1;
    }

    err = nvs_get_blob(nvs, NVS_KEY, &configblock, &size);

    if (err != ESP_OK || size != sizeof(configblock)) {
        configblockResetToDefaults(&configblock);
        configblockWrite(&configblock);
        return -1;
    }

    if (!configblockValidate(&configblock)) {
        configblockResetToDefaults(&configblock);
        configblockWrite(&configblock);
    }
    isInit = true;
    return 0;
}

bool configblockTest(void)
{
  return true;
}

static bool configblockCheckMagic(configblock_t *configblock)
{
  return (configblock->magic == MAGIC);
}

static bool configblockCheckVersion(configblock_t *configblock)
{
  return (configblock->version == VERSION);
}

static bool configblockCheckChecksum(configblock_t *configblock)
{
  return (configblock->cksum == calculate_cksum(configblock, sizeof(configblock_t) - 1));
}

static bool configblockCheckDataIntegrity(uint8_t *data, uint8_t version)
{
  bool status = false;

  if (version == 0)
  {
    struct configblock_v0_s *v0 = ( struct configblock_v0_s *)data;
    status = (v0->cksum == calculate_cksum(data, sizeof(struct configblock_v0_s) - 1));
  }
  else if (version == 1)
  {
    struct configblock_v1_s *v1 = ( struct configblock_v1_s *)data;
    status = (v1->cksum == calculate_cksum(data, sizeof(struct configblock_v1_s) - 1));
  }

  return status;
}

static bool configblockWrite(configblock_t *configblock)
{
    esp_err_t err;

    err = nvs_set_blob(
        nvs,
        "block",
        configblock,
        sizeof(configblock_t)
    );

    if (err != ESP_OK)
        return false;

    nvs_commit(nvs);
    return true;

  // // Write default configuration to eeprom
  // configblock->cksum = calculate_cksum(configblock, sizeof(configblock_t) - 1);
  // if (!eepromWriteBuffer((uint8_t *)configblock, 0, sizeof(configblock_t)))
  // {
  //   return false;
  // }

  // return true;
}

static bool configblockCopyToNewVersion(configblock_t *configblockSaved, configblock_t *configblockNew)
{
  configblock_t configblockTmp;

  // Copy new data to temp config memory
  memcpy((uint8_t *)&configblockTmp, (uint8_t *)configblockNew, sizeof(configblock_t));

  if (configblockSaved->version <= VERSION &&
      sizeof(configblock_t) >= configblockSizes[configblockSaved->version])
  {
    // Copy old saved eeprom data to new structure
    memcpy((uint8_t *)&configblockTmp + HEADER_SIZE_BYTES,
           (uint8_t *)configblockSaved + HEADER_SIZE_BYTES,
           configblockSizes[configblockSaved->version] - OVERHEAD_SIZE_BYTES);
    // Copy updated block to saved structure
    memcpy((uint8_t *)configblockSaved, (uint8_t *)&configblockTmp, sizeof(configblock_t));
  }
  else
  {
    return false;
  }

  return true;
}

/* Static accessors */
int configblockGetRadioChannel(void)
{
  if (cb_ok)
    return configblock.radioChannel;
  else
    return RADIO_CHANNEL;
}

int configblockGetRadioSpeed(void)
{
  if (cb_ok)
    return configblock.radioSpeed;
  else
    return RADIO_DATARATE;
}

uint64_t configblockGetRadioAddress(void)
{
  if (cb_ok)
    return ((uint64_t)configblock.radioAddress_upper << 32) | (uint64_t)configblock.radioAddress_lower;
  else
    return RADIO_ADDRESS;
}

float configblockGetCalibPitch(void)
{
  if (cb_ok)
    return configblock.calibPitch;
  else
    return 0;
}

float configblockGetCalibRoll(void)
{
  if (cb_ok)
    return configblock.calibRoll;
  else
    return 0;
}
