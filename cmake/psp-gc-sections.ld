/*
 * Preserve PSP loader metadata while --gc-sections removes unused game and
 * static-library code. The stock PSP linker script does not mark these input
 * sections KEEP(), so the NID table otherwise disappears before
 * psp-fixup-imports runs.
 */
SECTIONS
{
  .sceStub.text :
  {
    KEEP(*(.sceStub.text))
    KEEP(*(SORT(.sceStub.text.*)))
  }
  .lib.ent.top : { KEEP(*(.lib.ent.top)) }
  .lib.ent : { KEEP(*(.lib.ent)) }
  .lib.ent.btm : { KEEP(*(.lib.ent.btm)) }
  .lib.stub.top : { KEEP(*(.lib.stub.top)) }
  .lib.stub : { KEEP(*(.lib.stub)) }
  .lib.stub.btm : { KEEP(*(.lib.stub.btm)) }
  .rodata.sceModuleInfo : { KEEP(*(.rodata.sceModuleInfo)) }
  .rodata.sceResident : { KEEP(*(.rodata.sceResident)) }
  .rodata.sceNid : { KEEP(*(.rodata.sceNid)) }
  .rodata.sceVstub :
  {
    KEEP(*(.rodata.sceVstub))
    KEEP(*(SORT(.rodata.sceVstub.*)))
  }
}
INSERT AFTER .fini;
