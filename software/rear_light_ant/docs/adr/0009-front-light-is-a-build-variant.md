# The front light is a build variant of the rear light firmware

The front light uses the rear light's board with a white LED. Instead of a second firmware tree, `software/rear_light_ant` builds both: `make` is the rear light, `make LIGHT=front` the front light. About 95 % of the code (drivers, state machine, Smart with parking, battery, ANT+ transport and pages) is identical, and fixes such as the timer wake fix of the parking work must reach both lights. A copy would have to be kept in sync by hand. The BLE front light in `software/front_light_central` is superseded.

- **Where the variants differ.** Only in `src/variant.h` (brightness levels, brake boost, ANT+ light type, model number), the Mode Cycle in `src/sm.c` and the ANT+ mode map in `src/bike_light.c`, each chosen with `LIGHT_FRONT` next to its single configuration point. SPEC.md section 0 lists the differences. New per-light behaviour should be expressed as a variant constant, not as `#if LIGHT_FRONT` spread through the code.
- **Front modes.** Standard is the Steady state at the front's Base (30 %), Bright is a new state at Peak (80 %). The front light has no Flash, and front Smart has no brake boost: a brighter headlight while braking warns nobody and could dazzle oncoming traffic.
- **Lamp modes are not states any more.** Adding Bright broke the old rule that an ANT+ lamp mode number equals the state number (lamp 4 would have been Idle Charging). `main.c` now maps explicitly in both directions.
- **Both variants are tested.** `make test` runs the host tests once per variant; variant-specific expectations sit next to each other in the tests.
- **The directory keeps its name** (`rear_light_ant`) for now, to keep history and links. Rename it when the front light is in use.
- **Two lights on one bike computer.** Each light acts as the main light of its own network (ANT-6, no shared channel). A Wahoo controls the rear and the front light like this at the same time (VERIFY.md, F5).
