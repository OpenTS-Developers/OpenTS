---
key: IsMobileEMP
summary: Makes a vehicle build up an EM pulse charge and discharge it where it stands when it is deployed.
see_also: ["system:emp-pulse"]
when_omitted:
  kind: value
  value: "no"
---

The flag is what allows the deploy order at all for a vehicle with no [`DeploysInto`](/keys/deploysinto/) type and no passenger capacity. The deploy cursor is refused while the charge is short of [`MaxCharge`](/keys/maxcharge/). Deploying [fires the hard-coded pulse weapon](/systems/emp-pulse/#mobile-emp-vehicle) at the vehicle's own position and returns it to guard.
