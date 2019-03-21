#include "avr_compiler.h"
#include "TWI_master.h"
#include <util/delay.h>





//#define READ_SDA() (PORTK.IN & 0x80)
//#define READ_SCL() (PORTK.IN & 0x40)

/*! \brief Delay used to generate clock */
#define DELAY 2

/*! \brief Delay used for STOP condition */
#define SCL_SDA_DELAY 1

char twi_send_slave_address(twi_iface_t *iface, unsigned char slave_address, unsigned char read);

void twi_write_scl(twi_iface_t *iface, char x);
void twi_write_sda(twi_iface_t *iface, char x);
char twi_write_byte(twi_iface_t *iface,unsigned char byte);
char twi_read_byte(twi_iface_t *iface, unsigned char* rcvdata, unsigned char ack);


void delay_cycles(unsigned char i)
{
  volatile unsigned j;
  for(j=0;j<i;j++);
}
/*! \brief initialize twi master mode
 */ 
void twi_init(twi_iface_t *iface)
{
//  PORTK.DIRSET = 0xc0;
  twi_write_sda(iface,1);
  twi_write_scl(iface,1);
} 


/*! \brief Sends start condition
 */
char twi_start_cond(twi_iface_t *iface)
{
  delay_cycles(DELAY);
  twi_write_sda(iface,0);
  delay_cycles(DELAY);
  twi_write_scl(iface,0);  
  delay_cycles(DELAY);
  return 0;
}

char twi_stop_cond(twi_iface_t *iface)
{
  delay_cycles(DELAY);
  twi_write_scl(iface,1);
  delay_cycles(DELAY);
  twi_write_sda(iface,1);
  delay_cycles(DELAY);
  return 0;
}

char twi_send_slave_address(twi_iface_t *iface, unsigned char slave_address, unsigned char read)
{
  return twi_write_byte(iface,slave_address<<1 | read );
} 

char twi_write_data(twi_iface_t *iface, unsigned char slave_address, unsigned char* indata, char bytes)
{
  unsigned char index, ack = 0;
  
  if(twi_start_cond(iface))
    return 1;
  if(twi_send_slave_address(iface, slave_address, WRITE))
    return 2;  
  
  for(index = 0; index < bytes; index++)
  {
     ack = twi_write_byte(iface,indata[index]);
     if(ack)
      break;    
  }
  //put stop here
  twi_stop_cond(iface);
  if (ack) return 3;
  return 0;
  
}

char twi_write_byte(twi_iface_t *iface, unsigned char byte)
{
  char bit;
  for (bit = 0; bit < 8; bit++) 
  {
     twi_write_sda(iface,(byte & 0x80) != 0);
     delay_cycles(DELAY);
     twi_write_scl(iface,1);//goes high
     delay_cycles(DELAY);
     twi_write_scl(iface,0);//goes low
     byte <<= 1;
     delay_cycles(DELAY);
  }
  //release SDA
  twi_write_sda(iface,1);
  delay_cycles(DELAY);
 
  twi_write_scl(iface,1);//goes high
  //Check for acknowledgment
  char sda=(PORTK.IN & iface->sda);
  delay_cycles(DELAY);
  twi_write_scl(iface,0);//goes low
  return sda;
}  
char twi_read_data(twi_iface_t *iface, unsigned char slave_address,unsigned char* data, char bytes)
{
  unsigned char index,err,ack = 0;
  if(twi_start_cond(iface))
    return 1;
  if(twi_send_slave_address(iface,slave_address,READ))
    return 2;  
  for(index = 0; index < bytes; index++)
  {
    ack=0;
    if (index==bytes-1)ack=1;

    err = twi_read_byte(iface,&data[index], ack);
    if(err)
      break; 
  }
  //put stop here
  twi_stop_cond(iface);
  return err;
  
  
}  

/*! \brief Reads one byte into buffer.
    \param rcvdata Pointer to data buffer
    \param bytes  Number of bytes to read
    \param index Position of the incoming byte in hte receive buffer 
    \return 1 if successful, otherwise 0
 */
char twi_read_byte(twi_iface_t *iface, unsigned char* rcvdata, unsigned char ack)
{
  unsigned char byte = 0;
  unsigned char bit = 0;
  //release SDA
  twi_write_sda(iface,1);
  for (bit = 0; bit < 8; bit++) 
  {
    twi_write_scl(iface,1);//goes high
    if(PORTK.IN&iface->sda)
      byte|= (1 << (7- bit));
    delay_cycles(DELAY);
    twi_write_scl(iface,0);//goes low
    delay_cycles(DELAY);
  }
  *rcvdata = byte;

  twi_write_sda(iface,ack);
  twi_write_scl(iface,1); //goes high for the 9th clock
  delay_cycles(DELAY);
  //Pull SCL low
  twi_write_scl(iface,0); //end of byte with acknowledgment. 
  //release SDA
  twi_write_sda(iface,1);
  delay_cycles(DELAY);
  return 0;
}  
/*! \brief Writes SCL.
    \param x tristates SCL when x = 1, other wise 0
 */
void twi_write_scl (twi_iface_t *iface, char x)
{
      if(x)
      {
            PORTK.OUTSET = iface->scl; //pullup
            PORTK.DIRCLR = iface->scl; //tristate it
            //check clock stretching
            while(!(PORTK.IN & iface->scl));
//            PORTK.DIRSET = 0x40; //output 
//            PORTK.OUTSET = 0x40; //set it low

      }
      else
      {
            PORTK.DIRSET = iface->scl; //output 
            PORTK.OUTCLR = iface->scl; //set it low
      }
}

/*! \brief Writes SDA.
    \param x tristates SDA when x = 1, other wise 0
 */
void twi_write_sda (twi_iface_t *iface, char x)
{
      if(x)
      {
            PORTK.DIRCLR = iface->sda; //tristate it
            PORTK.OUTSET = iface->sda; //pull up

//            PORTK.DIRSET = 0x80; //tristate it
//            PORTK.OUTSET = 0x80; //pull up
      }
      else
      {
            PORTK.DIRSET = iface->sda; //output 
            PORTK.OUTCLR = iface->sda; //set it low
            
      }
}

