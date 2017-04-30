let SessionLoad = 1
if &cp | set nocp | endif
let s:so_save = &so | let s:siso_save = &siso | set so=0 siso=0
let v:this_session=expand("<sfile>:p")
silent only
cd ~/projects/voxel_cone_tracing
if expand('%') == '' && !&modified && line('$') <= 1 && getline(1) == ''
  let s:wipebuf = bufnr('%')
endif
set shortmess=aoO
badd +1 src/renderer.cpp
badd +36 shader/passthrough.frag
badd +2 shader/passthrough.vert
badd +17 src/pass.h
badd +1 lib/gl_utils/include/gl_utils/gl_helpers.h
badd +121 src/renderer.h
badd +1 src/camera.h
badd +70 src/main.cpp
badd +30 shader/voxelize.vert
badd +27 shader/voxelize.geom
badd +1 src/device.cpp
badd +15 src/command_bucket.h
badd +21 shader/voxel_cone_tracing.vert
badd +35 shader/voxel_cone_tracing.frag
badd +42 shader/voxelize.frag
badd +81 lib/gl_utils/src/gl_helpers.c
argglobal
silent! argdel *
argadd src/renderer.cpp
edit src/main.cpp
set splitbelow splitright
wincmd _ | wincmd |
vsplit
1wincmd h
wincmd _ | wincmd |
split
1wincmd k
wincmd _ | wincmd |
vsplit
1wincmd h
wincmd w
wincmd w
wincmd _ | wincmd |
vsplit
1wincmd h
wincmd w
wincmd w
set nosplitbelow
set nosplitright
wincmd t
set winheight=1 winwidth=1
exe '1resize ' . ((&lines * 33 + 34) / 68)
exe 'vert 1resize ' . ((&columns * 84 + 127) / 254)
exe '2resize ' . ((&lines * 33 + 34) / 68)
exe 'vert 2resize ' . ((&columns * 84 + 127) / 254)
exe '3resize ' . ((&lines * 32 + 34) / 68)
exe 'vert 3resize ' . ((&columns * 84 + 127) / 254)
exe '4resize ' . ((&lines * 32 + 34) / 68)
exe 'vert 4resize ' . ((&columns * 84 + 127) / 254)
exe 'vert 5resize ' . ((&columns * 84 + 127) / 254)
argglobal
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 76 - ((21 * winheight(0) + 16) / 33)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
76
normal! 0
wincmd w
argglobal
edit shader/voxelize.frag
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 64 - ((26 * winheight(0) + 16) / 33)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
64
normal! 03|
wincmd w
argglobal
edit shader/voxelize.vert
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 25 - ((23 * winheight(0) + 16) / 32)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
25
normal! 0
wincmd w
argglobal
edit shader/voxelize.geom
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 2 - ((0 * winheight(0) + 16) / 32)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
2
normal! 0
wincmd w
argglobal
edit src/renderer.cpp
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal nofen
silent! normal! zE
let s:l = 294 - ((40 * winheight(0) + 33) / 66)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
294
normal! 024|
wincmd w
exe '1resize ' . ((&lines * 33 + 34) / 68)
exe 'vert 1resize ' . ((&columns * 84 + 127) / 254)
exe '2resize ' . ((&lines * 33 + 34) / 68)
exe 'vert 2resize ' . ((&columns * 84 + 127) / 254)
exe '3resize ' . ((&lines * 32 + 34) / 68)
exe 'vert 3resize ' . ((&columns * 84 + 127) / 254)
exe '4resize ' . ((&lines * 32 + 34) / 68)
exe 'vert 4resize ' . ((&columns * 84 + 127) / 254)
exe 'vert 5resize ' . ((&columns * 84 + 127) / 254)
tabnext 1
if exists('s:wipebuf')
  silent exe 'bwipe ' . s:wipebuf
endif
unlet! s:wipebuf
set winheight=1 winwidth=20 shortmess=filnxtToO
let s:sx = expand("<sfile>:p:r")."x.vim"
if file_readable(s:sx)
  exe "source " . fnameescape(s:sx)
endif
let &so = s:so_save | let &siso = s:siso_save
let g:this_session = v:this_session
let g:this_obsession = v:this_session
let g:this_obsession_status = 2
doautoall SessionLoadPost
unlet SessionLoad
" vim: set ft=vim :
