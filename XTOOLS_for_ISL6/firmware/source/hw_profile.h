#ifndef __HW_PROFILE_H__
#define __HW_PROFILE_H__

#define HW_MODE_330   1
#define HW_MODE_430P  2

void HWProfile_Select(void);
unsigned char HWProfile_GetMode(void);
char *HWProfile_GetModelText(void);

#endif
