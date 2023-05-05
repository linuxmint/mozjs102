//! RISC-V Settings.

use crate::settings::{self, detail, Builder, Value};
use core::fmt;

// Include code generated by `cranelift-codegen/meta/src/gen_settings.rs`. This file contains a
// public `Flags` struct with an impl for all of the settings defined in
// `cranelift-codegen/meta/src/isa/riscv/mod.rs`.
include!(concat!(env!("OUT_DIR"), "/settings-riscv.rs"));

#[cfg(test)]
mod tests {
    use super::{builder, Flags};
    use crate::settings::{self, Configurable};
    use alloc::string::ToString;

    #[test]
    fn display_default() {
        let shared = settings::Flags::new(settings::builder());
        let b = builder();
        let f = Flags::new(&shared, b);
        assert_eq!(
            f.to_string(),
            "[riscv]\n\
             supports_m = false\n\
             supports_a = false\n\
             supports_f = false\n\
             supports_d = false\n\
             enable_m = true\n\
             enable_e = false\n"
        );
        // Predicates are not part of the Display output.
        assert_eq!(f.full_float(), false);
    }

    #[test]
    fn predicates() {
        let mut sb = settings::builder();
        sb.set("enable_simd", "true").unwrap();
        let shared = settings::Flags::new(sb);
        let mut b = builder();
        b.enable("supports_f").unwrap();
        b.enable("supports_d").unwrap();
        let f = Flags::new(&shared, b);
        assert_eq!(f.full_float(), true);

        let mut sb = settings::builder();
        sb.set("enable_simd", "false").unwrap();
        let shared = settings::Flags::new(sb);
        let mut b = builder();
        b.enable("supports_f").unwrap();
        b.enable("supports_d").unwrap();
        let f = Flags::new(&shared, b);
        assert_eq!(f.full_float(), false);
    }
}
