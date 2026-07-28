# CoAkka Logger

CoAkka Logger is for applications that need logging behavior to stay explicit
under load. It gives the host language a small logger API while the native
logger core owns queue shape, pressure behavior, drain semantics, counters, and
lifecycle state.

## The Boundary It Improves

Logging often looks harmless until the system is already stressed. Many
logging stacks become unreliable exactly when operators need them most:

- synchronous sinks stall request, worker, or event-loop paths
- asynchronous appenders hide queue growth until RAM and latency spike
- implicit drop behavior loses records without a clean emitted, delivered,
  dropped, or rejected account
- language-by-language adapters drift in semantics, counters, and diagnostics

The common shape is:

```text
app code -> async appender -> hidden queue growth or silent loss
```

CoAkka Logger is designed to be narrower and more predictable:

```text
app code -> bounded native logger -> accepted, delivered, dropped, and drained evidence
```

The useful claim is not "another logging facade" and not "log output looks
prettier." The useful claim is that the host can observe queue capacity,
accepted records, delivered records, dropped records, rejected writes, drain
state, and native version information in the same shape across languages.

## What The Logger Owns

| Concern | Logger role |
| --- | --- |
| Bounded queue | Keep log intake bounded instead of pretending memory is infinite. |
| Pressure behavior | Reject or drop explicitly according to the configured logger policy. |
| Counters | Report emitted, delivered, dropped, queue depth, and high-water marks. |
| Drain semantics | Let samples and embedding tests read records deterministically. |
| Native identity | Report native ABI, version, and git generation for support and release checks. |

The host app still owns log categories, message content, redaction policy,
business correlation IDs, sink selection, and operational rollout.

## Runtime And Logger Together

Runtime and logger are separate package lanes, but they complement each other.
Runtime explains where work went:

```text
target -> handler -> reply or deadletter
```

Logger explains what the host emitted and what happened under pressure:

```text
category + message -> bounded logger -> drain and counters
```

Together they give a service boundary both delivery evidence and observability
evidence without forcing every language to reinvent queue behavior.

## Why It Matters

CoAkka Logger is useful when:

- async logging hides queue growth until latency or memory spikes
- loss under pressure is possible but hard to measure
- each language adapter reports different counters
- synchronous sink behavior can stall hot request or worker paths
- native-backed runtime diagnostics should share one operational vocabulary
- tests need deterministic drain behavior instead of scraping console output

Keep a simpler logger when the app is small, one language is all that matters,
and existing logging behavior is already bounded and easy to audit.

## Read Next

- [Runnable logger samples](https://github.com/phuong-tran/coakka-samples/tree/main/logger)
- [Production readiness](https://github.com/phuong-tran/coakka-samples/blob/main/docs/production-readiness.md)
- [Compatibility matrix](https://github.com/phuong-tran/coakka-publish/blob/main/docs/compatibility-matrix.md)
- [Ecosystem map](coakka-ecosystem.md)
