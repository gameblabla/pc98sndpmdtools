#ifndef PMD_H
#define PMD_H

#define PMD_VECTOR 0x60
#define PCM86_VECTOR 0x65

extern void call_pmd_vector();
extern int check_pmd();
extern int check_pcm86();
extern void CallMusicDriver(unsigned char ah_value, unsigned int *segment, unsigned int *offset);
extern void pmd_mstart(const unsigned char *music_data, unsigned int music_data_size);
extern int initialize_p86();
extern int load_p86_file(const char *filename);
extern void pmd_play_FM_sound_effect(unsigned char sound_effect_index);
extern void pmd_play_pcm_sound_effect(unsigned char sound_effect_index, unsigned short frequency, char pan, unsigned char volume);

#endif
