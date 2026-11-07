# Rear Light Peripheral

> **Superseded.** The rear light is now controlled over ANT+ by the firmware in [`../rear_light_ant`](../rear_light_ant) (ADR-0005 there). This Zephyr/BLE firmware is kept as the behavioural reference and for the BLE front light, until the physical checks in `../rear_light_ant/VERIFY.md` are done; it is not developed any further.

Firmware for the bicycle rear light (nRF52833, Zephyr / nRF Connect SDK). It is the BLE peripheral that the front light central talks to.

## Where to read what

| File | Purpose |
| --- | --- |
| [MANUAL.md](./MANUAL.md) | User-facing manual: modes, buttons, charging, BLE. |
| [CONTEXT.md](./CONTEXT.md) | Glossary. The words used in code, spec and manual (Off, Deep Sleep, Base, Peak, ...). Read this first. |
| [SPEC.md](./SPEC.md) | Behavioural requirements ("shall" statements with IDs). It names code constants and does not repeat their values: the code is the single source of truth for numbers. |
| [docs/adr/](./docs/adr) | Architecture Decision Records, see below. |

## What is an ADR?

An Architecture Decision Record is a short note (often one paragraph) that records **why** a decision was made. It is written only for decisions that are hard to reverse, surprising when reading the code, and the result of a real trade-off. ADRs exist so that nobody "fixes" a deliberate choice without knowing the reasoning.

- Files are numbered in order: `docs/adr/0001-short-title.md`.
- Do not rewrite history. If a decision changes, add a new ADR that supersedes the old one.
- Plain parameter choices (thresholds, timings) do not need an ADR. They belong in the code and the spec.

## Where this directory fits

These docs cover **only** the rear light firmware. Other projects under `software/` (such as the front light central) keep their own documentation.
