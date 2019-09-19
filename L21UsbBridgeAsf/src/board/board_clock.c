#include <asf.h>

void configure_gclock_generator(void)
{
//! [setup_1]
	struct system_gclk_gen_config gclock_gen_conf;
//! [setup_1]
//! [setup_2]
	system_gclk_gen_get_config_defaults(&gclock_gen_conf);
//! [setup_2]

#if (SAML21) || (SAML22) || (SAMR30)
//! [setup_3]
	gclock_gen_conf.source_clock    = SYSTEM_CLOCK_SOURCE_OSC16M;
	gclock_gen_conf.division_factor = 128;
//! [setup_3]
#elif (SAMC21)
//! [setup_3]
	gclock_gen_conf.source_clock    = SYSTEM_CLOCK_SOURCE_OSC48M;
	gclock_gen_conf.division_factor = 128;
//! [setup_3]
#elif (SAMD)
	gclock_gen_conf.source_clock    = SYSTEM_CLOCK_SOURCE_DFLL;
	gclock_gen_conf.division_factor = 128;
#else
//! [setup_3]
	gclock_gen_conf.source_clock    = SYSTEM_CLOCK_SOURCE_OSC8M;
	gclock_gen_conf.division_factor = 128;
//! [setup_3]
#endif

//! [setup_4]
	system_gclk_gen_set_config(GCLK_GENERATOR_1, &gclock_gen_conf);
//! [setup_4]

//! [setup_5]
	system_gclk_gen_enable(GCLK_GENERATOR_1);
//! [setup_5]
}