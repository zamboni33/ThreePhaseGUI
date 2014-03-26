

#define MIN_FREQUENCY 20

void set_VFTable(char vinput[16], char linput[16]);
void set_VLITable(char vinput[16], char linput[16]);
void voltage_calc(unsigned short equation); 
void voltage_calctable(void); 
int frequency_calctable(long float voltage); 
void photo_update(unsigned short AD_photo_new, unsigned short AD_photo_old);
void RPM_calc(unsigned short current_frequency);
void current_calc(void);
void power_calculate(unsigned short current_frequency);
/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

