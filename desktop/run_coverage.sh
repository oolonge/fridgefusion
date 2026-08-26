#!/bin/bash

func_test_output=$(./build/Desktop_arm_darwin_generic_mach_o_64bit-Debug/tests/tests)
llvm-profdata merge -sparse default.profraw -o default.profdata
llvm_export_output=$(llvm-cov export ./build/Desktop_arm_darwin_generic_mach_o_64bit-Debug/tests/tests -instr-profile=default.profdata -ignore-filename-regex=".*/(ui_|moc_).*")
llvm_report_output=$(llvm-cov report ./build/Desktop_arm_darwin_generic_mach_o_64bit-Debug/tests/tests -instr-profile=default.profdata -ignore-filename-regex=".*/(ui_|moc_).*")
percent=$(echo "$llvm_export_output" | jq '.data[].totals.lines.percent')

printf "Coverage: %.2f%%\n\n" "$percent"
echo "LLVM-COV REPORT:"
echo "$llvm_report_output"

echo "TESTS OUTPUT:"
echo "$func_test_output"
