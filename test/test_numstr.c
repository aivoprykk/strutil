#include "../include/numstr.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

void test_sec_to_hms_str() {
    char buffer[32];
    
    printf("Testing sec_to_hms_str function:\n");
    printf("=== Testing with hide_sec=false (full h:m:s format) ===\n");
    
    // Test case 1: Only seconds (< 60 seconds)
    sec_to_hms_str(45, buffer, false);
    printf("45 seconds: '%s'\n", buffer);
    assert(strcmp(buffer, "00:00:45") == 0);
    
    // Test case 2: Zero seconds
    sec_to_hms_str(0, buffer, false);
    printf("0 seconds: '%s'\n", buffer);
    assert(strcmp(buffer, "00:00:00") == 0);
    
    // Test case 3: Minutes and seconds (no hours)
    sec_to_hms_str(125, buffer, false); // 2:05
    printf("125 seconds (2m 5s): '%s'\n", buffer);
    assert(strcmp(buffer, "00:02:05") == 0);
    
    // Test case 4: Exactly 1 minute (no seconds)
    sec_to_hms_str(60, buffer, false);
    printf("60 seconds (1m 0s): '%s'\n", buffer);
    assert(strcmp(buffer, "00:01:00") == 0);
    
    // Test case 5: Multiple minutes, no seconds
    sec_to_hms_str(300, buffer, false); // 5:00
    printf("300 seconds (5m 0s): '%s'\n", buffer);
    assert(strcmp(buffer, "00:05:00") == 0);
    
    // Test case 6: Hours, minutes, and seconds
    sec_to_hms_str(3725, buffer, false); // 1:02:05
    printf("3725 seconds (1h 2m 5s): '%s'\n", buffer);
    assert(strcmp(buffer, "01:02:05") == 0);
    
    // Test case 7: Exactly 1 hour (no minutes or seconds)
    sec_to_hms_str(3600, buffer, false);
    printf("3600 seconds (1h 0m 0s): '%s'\n", buffer);
    assert(strcmp(buffer, "01:00:00") == 0);
    
    // Test case 8: Hours and seconds (no minutes)
    sec_to_hms_str(3605, buffer, false); // 1:00:05
    printf("3605 seconds (1h 0m 5s): '%s'\n", buffer);
    assert(strcmp(buffer, "01:00:05") == 0);
    
    // Test case 9: Hours and minutes (no seconds)
    sec_to_hms_str(3720, buffer, false); // 1:02:00
    printf("3720 seconds (1h 2m 0s): '%s'\n", buffer);
    assert(strcmp(buffer, "01:02:00") == 0);
    
    // Test case 10: Multiple hours
    sec_to_hms_str(7325, buffer, false); // 2:02:05
    printf("7325 seconds (2h 2m 5s): '%s'\n", buffer);
    assert(strcmp(buffer, "02:02:05") == 0);
    
    // Test case 11: Large number (24+ hours)
    sec_to_hms_str(90061, buffer, false); // 25:01:01
    printf("90061 seconds (25h 1m 1s): '%s'\n", buffer);
    assert(strcmp(buffer, "25:01:01") == 0);
    
    printf("\n=== Testing with hide_sec=true (two-segment format) ===\n");
    
    // Test case 12: Only seconds (< 60 seconds) - should show m:s
    sec_to_hms_str(45, buffer, true);
    printf("45 seconds (hide_sec=true): '%s'\n", buffer);
    assert(strcmp(buffer, "00:45") == 0);
    
    // Test case 13: Zero seconds
    sec_to_hms_str(0, buffer, true);
    printf("0 seconds (hide_sec=true): '%s'\n", buffer);
    assert(strcmp(buffer, "00:00") == 0);
    
    // Test case 14: Minutes and seconds (no hours)
    sec_to_hms_str(125, buffer, true); // 2:05
    printf("125 seconds (hide_sec=true): '%s'\n", buffer);
    assert(strcmp(buffer, "02:05") == 0);
    
    // Test case 15: Exactly 1 minute (no seconds)
    sec_to_hms_str(60, buffer, true);
    printf("60 seconds (hide_sec=true): '%s'\n", buffer);
    assert(strcmp(buffer, "01:00") == 0);
    
    // Test case 16: Hours, minutes, and seconds - should show h:m
    sec_to_hms_str(3725, buffer, true); // 1:02:05
    printf("3725 seconds (hide_sec=true): '%s'\n", buffer);
    assert(strcmp(buffer, "01:02") == 0);
    
    // Test case 17: Exactly 1 hour (no minutes or seconds)
    sec_to_hms_str(3600, buffer, true);
    printf("3600 seconds (hide_sec=true): '%s'\n", buffer);
    assert(strcmp(buffer, "01:00") == 0);
    
    // Test case 18: Large number with hours
    sec_to_hms_str(90061, buffer, true); // 25:01:01
    printf("90061 seconds (hide_sec=true): '%s'\n", buffer);
    assert(strcmp(buffer, "25:01") == 0);
    
    printf("All sec_to_hms_str tests passed!\n\n");
}

int main() {
    // Test cases for numstr functions
    char buffer[32];
    
    test_sec_to_hms_str();
    
    return 0;
}