#include "eeprom.h"
#include "config.h"
#include <cstring>
#include <mutex>


struct EepromData {
    struct {
        uint16_t address;
        uint16_t ppm_target;
        uint8_t reserved[4];
        char ssid[NETWORK_SSID_MAX_LENGTH];
        char password[NETWORK_PASSWORD_MAX_LENGTH];
        uint16_t checksum;
    } page;
    struct {
        uint16_t ppm_target;
        char ssid[NETWORK_SSID_MAX_LENGTH];
        char password[NETWORK_PASSWORD_MAX_LENGTH];
    } tmp;
};
static_assert(sizeof(EepromData::page) == EEPROM_ADDRESS_SIZE + EEPROM_PAGE_SIZE);

void Eeprom::task_entry(void *param)
{
    static_cast<Eeprom *>(param)->task();
}

Eeprom::Eeprom(PicoI2C &i2c)
: m_i2c(i2c)
, m_should_flush(xSemaphoreCreateBinary())
, m_flush_mutex()
, m_access_mutex()
, m_data(std::make_unique<EepromData>())
{
    memset(m_data.get(), 0, sizeof(EepromData));
    m_data->page.address = 0x0000;
    xTaskCreate(task_entry, "EEPROM", 256, this, TASK_EEPROM_PRIORITY, nullptr);
}

Eeprom::~Eeprom() = default;

static uint16_t crc16(const uint8_t *buffer, size_t length) {
    uint8_t x;
    uint16_t crc = 0xFFFF;
    while (length--) {
        x = crc >> 8 ^ *buffer++;
        x ^= x >> 4;
        crc = (crc << 8) ^ ((uint16_t) (x << 12)) ^ ((uint16_t) (x << 5)) ^ ((uint16_t) x);
    }
    return crc;
}

static void eeprom_load_defaults(EepromData *data) {
    data->page.ppm_target = PPM_TARGET_DEFAULT;
    strncpy(data->page.ssid, NETWORK_SSID_DEFAULT, NETWORK_SSID_MAX_LENGTH);
    strncpy(data->page.password, NETWORK_SSID_DEFAULT, NETWORK_PASSWORD_MAX_LENGTH);
    data->tmp.ppm_target = data->page.ppm_target;
    strncpy(data->tmp.ssid, data->page.ssid, NETWORK_SSID_MAX_LENGTH);
    strncpy(data->tmp.password, data->page.password, NETWORK_PASSWORD_MAX_LENGTH);
}

void Eeprom::LoadBlocking() {
    std::lock_guard<Fmutex> flush_guard { m_flush_mutex };
    std::lock_guard<Fmutex> access_guard { m_access_mutex };
    uint8_t *ptr = reinterpret_cast<uint8_t*>(&m_data->page);
    uint processed = m_i2c.transaction(EEPROM_DEVICE_ADDRESS, ptr, EEPROM_ADDRESS_SIZE, ptr + EEPROM_ADDRESS_SIZE, EEPROM_PAGE_SIZE);
    if (processed != EEPROM_ADDRESS_SIZE + EEPROM_PAGE_SIZE) {
        printf("Failed to read from EEPROM\n");
        eeprom_load_defaults(m_data.get());
        return;
    }

    printf("PPM  : %u\n", +m_data->page.ppm_target);
    printf("SSID : %.*s\n", NETWORK_SSID_MAX_LENGTH, m_data->page.ssid);

    uint16_t crc = crc16(ptr + 2, EEPROM_PAGE_SIZE);
    if (crc != 0) {
        printf("Invalid CRC: got:0x%04x expected:0x%04x (stored:0x%04x)\n", +crc, 0, +m_data->page.checksum);
        eeprom_load_defaults(m_data.get());
        return;
    }

    m_data->tmp.ppm_target = m_data->page.ppm_target;
    strncpy(m_data->tmp.ssid, m_data->page.ssid, NETWORK_SSID_MAX_LENGTH);
    strncpy(m_data->tmp.password, m_data->page.password, NETWORK_PASSWORD_MAX_LENGTH);
}

static bool eeprom_should_flush(EepromData &data) {
    if (data.tmp.ppm_target != data.page.ppm_target)
        return true;
    if (strncmp(data.tmp.ssid, data.page.ssid, NETWORK_SSID_MAX_LENGTH) != 0)
        return true;
    if (strncmp(data.tmp.password, data.page.password, NETWORK_PASSWORD_MAX_LENGTH) != 0)
        return true;
    return false;
}

void Eeprom::task()
{
    while (true) {
        xSemaphoreTake(m_should_flush, portMAX_DELAY);
        std::lock_guard<Fmutex> flush_guard { m_flush_mutex };
        {
            std::lock_guard<Fmutex> access_guard { m_access_mutex };
            if (!eeprom_should_flush(*m_data)) {
                continue;
            }
            m_data->page.ppm_target = m_data->tmp.ppm_target;
            strncpy(m_data->page.ssid, m_data->tmp.ssid, NETWORK_SSID_MAX_LENGTH);
            strncpy(m_data->page.password, m_data->tmp.password, NETWORK_PASSWORD_MAX_LENGTH);
            uint8_t *ptr = reinterpret_cast<uint8_t*>(&m_data->page);
            uint16_t crc = crc16(ptr + 2, EEPROM_PAGE_SIZE - 2);
            m_data->page.checksum = crc << 8 | crc >> 8;
            uint wrote = m_i2c.write(EEPROM_DEVICE_ADDRESS, ptr, EEPROM_ADDRESS_SIZE + EEPROM_PAGE_SIZE);
            if (wrote != sizeof(EepromData::page))
                printf("Failed to write to EEPROM\n");
        }
        vTaskDelay(EEPROM_FLUSH_TIME_MS / portTICK_PERIOD_MS);
    }
}

void Eeprom::QueueTargetPPM(uint16_t ppm)
{
    std::lock_guard<Fmutex> access_guard { m_access_mutex };
    m_data->tmp.ppm_target = ppm;
    xSemaphoreGive(m_should_flush);
}

void Eeprom::QueueNetworkCredentials(const char *ssid, const char *password)
{
    std::lock_guard<Fmutex> access_guard { m_access_mutex };
    strncpy(m_data->tmp.ssid, ssid, NETWORK_SSID_MAX_LENGTH);
    strncpy(m_data->tmp.password, password, NETWORK_PASSWORD_MAX_LENGTH);
    xSemaphoreGive(m_should_flush);
}

