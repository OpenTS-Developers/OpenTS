---
key: CreditReserve
summary: The balance a computer house must hold before it will start repairing one of its buildings.
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: "1000"
---

The house's available money is compared against this figure once a damaged building has passed every other gate. At or above it the wrench goes on and the repair is paid for step by step out of the same treasury, which can take the balance below the reserve while the repair runs. The reserve gates the decision to start, not the spending that follows.

Below it the same routine considers [selling the building instead](/systems/repair/#when-the-computer-repairs). Raising the figure therefore makes a computer house both slower to repair and quicker to demolish what it cannot afford to keep.
