/* CPU DVFS through the K230 power-management API (mpi_pm_api.h, kernel RT_USING_PM): list the CPU domain's
 * frequency/voltage profiles, switch to manual governor at the lowest-frequency profile for 10 s, then restore. */
#include <stdio.h>
#include <unistd.h>
#include "mpi_pm_api.h"
#include "board.h"

int main(void)
{
    k_pm_profile prof[K_PM_MAX_PROFILE_COUNT];
    uint32_t count = K_PM_MAX_PROFILE_COUNT;
    k_pm_governor gov0;
    int32_t idx0 = -1;

    if (kd_mpi_pm_get_profiles(PM_DOMAIN_CPU, &count, prof) || count == 0) { printf("pm: no CPU profiles\n"); return 1; }
    kd_mpi_pm_get_governor(PM_DOMAIN_CPU, &gov0);
    kd_mpi_pm_get_profile(PM_DOMAIN_CPU, &idx0);
    printf("low-power (%s): governor %d, profile %d of %u\n", BOARD_NAME, (int)gov0, (int)idx0, (unsigned)count);
    int low = 0;
    for (uint32_t i = 0; i < count; i++) {
        printf("  [%u] %d Hz, %d uV\n", (unsigned)i, (int)prof[i].freq, (int)prof[i].volt);
        if (prof[i].freq < prof[low].freq) low = (int)i;
    }
    if (kd_mpi_pm_set_governor(PM_DOMAIN_CPU, PM_GOVERNOR_MANUAL) || kd_mpi_pm_set_profile(PM_DOMAIN_CPU, low)) {
        printf("pm: cannot set profile %d\n", low);
        return 1;
    }
    int32_t now = -1;
    kd_mpi_pm_get_profile(PM_DOMAIN_CPU, &now);
    printf("CPU at profile %d (%d Hz) for 10 s; the thread sleeps, so the core idles in WFI\n", (int)now, (int)prof[low].freq);
    sleep(10);
    if (idx0 >= 0) kd_mpi_pm_set_profile(PM_DOMAIN_CPU, idx0);
    kd_mpi_pm_set_governor(PM_DOMAIN_CPU, gov0);
    printf("restored governor %d, profile %d\n", (int)gov0, (int)idx0);
    return 0;
}
