# BLE control is unauthenticated

The control characteristic can be written by any BLE central in range, with no pairing, bonding or encryption, and the advertised device name is still the Zephyr placeholder. We kept it open because the only intended client is the front light central, and requiring pairing would complicate setup for little gain on a bike light. Anyone in range can therefore turn the light off or change its mode. Do not rely on BLE as a security boundary, and revisit this before adding any behaviour that needs trust.
