stack_size equ 0x2000

_start:
  la      $t0, _startup_data
  sw      $sp, 0x0000($t0)
  sw      $ra, 0x0004($t0)
  la      $sp, _stack + stack_size - 0x10
  jal     main
  nop
  la      $t0, _startup_data
  lw      $sp, 0x0000($t0)
  lw      $ra, 0x0004($t0)
  jr      $ra
  nop


gz_leave:
  addiu   $sp, $sp, -0x0008
  sw      $ra, 0x0004($sp)
  la      $t0, _startup_data
  lw      $t1, 0x0000($t0)
  sw      $sp, 0x0000($t0)
  move    $sp, $t1
  lw      $t9, 0x0008($t0)
  jalr    $t9
  nop
  la      $t0, _startup_data
  lw      $t1, 0x0000($t0)
  sw      $sp, 0x0000($t0)
  move    $sp, $t1
  lw      $ra, 0x0004($sp)
  jr      $ra
  addiu   $sp, $sp, 0x0008
