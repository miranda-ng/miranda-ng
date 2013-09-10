/* Default linker script, for normal executables */
OUTPUT_FORMAT(pei-i386)
SECTIONS
{
  /* Make the virtual address and file offset synced if the alignment is
     lower than the target page size. */
  . = SIZEOF_HEADERS;
  . = ALIGN(__section_alignment__);
 .rsrc __image_base__ + BLOCK(__section_alignment__) :
  {
    *(.rsrc)
    *(SORT(.rsrc$*))
  }
  /DISCARD/ : 
  {
    *(.edata)
  }   
}
