
# interface with asm, regs and cmd windows
define split
  layout split
  layout asm
  layout regs
  focus cmd
end

# interface with C source, regs and cmd windows
define ss
  layout split
  layout src
  layout regs
  focus cmd
end

define flash
  dont-repeat
  mon reset
  load
end


