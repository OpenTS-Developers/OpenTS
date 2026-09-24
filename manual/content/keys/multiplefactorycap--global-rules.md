---
key: MultipleFactoryCap
scope: global-rules
label: Limit the multiple-factory discount
summary: The most factories of one category that count toward the MultipleFactory build-time discount.
see_also: [MultipleFactory, "system:production"]
when_omitted:
  kind: value
  value: "0"
---

Factories of one category past this number no longer shorten its build times. At `MultipleFactoryCap=3`, [`MultipleFactory`](/keys/multiplefactory/) applies for the second and third factory and not for a fourth. A value of `1` turns the discount off, and `0` or below counts every factory.
