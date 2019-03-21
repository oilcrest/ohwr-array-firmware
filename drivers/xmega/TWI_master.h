#ifndef TWI_MASTER_H_
#define TWI_MASTER_H_



#define WRITE 0x0
#define READ 0x1

typedef struct twi_iface_struct
{
 uint8_t sda;
 uint8_t scl;
}
twi_iface_t;

void twi_init(twi_iface_t *iface);

char twi_start_cond(twi_iface_t *iface);
char twi_stop_cond(twi_iface_t *iface);
char twi_write_data(twi_iface_t *iface, unsigned char slave_address, unsigned char* data, char bytes);
char twi_read_data(twi_iface_t *iface, unsigned char slave_address, unsigned char* data, char bytes);



#endif /* TWI_MASTER_H_ */
