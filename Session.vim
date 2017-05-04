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
badd +87 src/renderer.cpp
badd +36 shader/passthrough.frag
badd +2 shader/passthrough.vert
badd +17 src/pass.h
badd +96 src/renderer.h
badd +135 src/main.cpp
badd +54 shader/voxelize.geom
badd +1 src/device.cpp
badd +15 src/command_bucket.h
badd +19 shader/voxel_cone_tracing.vert
badd +228 shader/voxel_cone_tracing.frag
badd +134 shader/voxelize.frag
badd +131 lib/gl_utils/src/gl_helpers.c
badd +28 shader/voxelize.vert
badd +8 src/texture_3d.h
badd +32 src/texture_3d.cpp
badd +7 CMakeLists.txt
badd +1 shader/mipmap.comp
argglobal
silent! argdel *
argadd src/renderer.cpp
edit shader/voxelize.frag
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
wincmd _ | wincmd |
split
1wincmd k
wincmd w
set nosplitbelow
set nosplitright
wincmd t
set winheight=1 winwidth=1
exe '1resize ' . ((&lines * 32 + 34) / 68)
exe 'vert 1resize ' . ((&columns * 84 + 127) / 254)
exe '2resize ' . ((&lines * 32 + 34) / 68)
exe 'vert 2resize ' . ((&columns * 84 + 127) / 254)
exe '3resize ' . ((&lines * 33 + 34) / 68)
exe 'vert 3resize ' . ((&columns * 84 + 127) / 254)
exe '4resize ' . ((&lines * 33 + 34) / 68)
exe 'vert 4resize ' . ((&columns * 84 + 127) / 254)
exe '5resize ' . ((&lines * 32 + 34) / 68)
exe 'vert 5resize ' . ((&columns * 84 + 127) / 254)
exe '6resize ' . ((&lines * 33 + 34) / 68)
exe 'vert 6resize ' . ((&columns * 84 + 127) / 254)
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
let s:l = 145 - ((26 * winheight(0) + 16) / 32)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
145
normal! 047|
wincmd w
argglobal
edit shader/mipmap.comp
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 39 - ((31 * winheight(0) + 16) / 32)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
39
normal! 0
wincmd w
argglobal
edit src/renderer.h
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 107 - ((15 * winheight(0) + 16) / 33)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
107
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
setlocal fen
silent! normal! zE
let s:l = 300 - ((12 * winheight(0) + 16) / 33)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
300
normal! 0
wincmd w
argglobal
edit shader/voxel_cone_tracing.frag
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 234 - ((25 * winheight(0) + 16) / 32)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
234
normal! 0
wincmd w
argglobal
edit src/main.cpp
setlocal fdm=manual
setlocal fde=0
setlocal fmr={{{,}}}
setlocal fdi=#
setlocal fdl=0
setlocal fml=1
setlocal fdn=20
setlocal fen
silent! normal! zE
let s:l = 76 - ((14 * winheight(0) + 16) / 33)
if s:l < 1 | let s:l = 1 | endif
exe s:l
normal! zt
76
normal! 040|
wincmd w
exe '1resize ' . ((&lines * 32 + 34) / 68)
exe 'vert 1resize ' . ((&columns * 84 + 127) / 254)
exe '2resize ' . ((&lines * 32 + 34) / 68)
exe 'vert 2resize ' . ((&columns * 84 + 127) / 254)
exe '3resize ' . ((&lines * 33 + 34) / 68)
exe 'vert 3resize ' . ((&columns * 84 + 127) / 254)
exe '4resize ' . ((&lines * 33 + 34) / 68)
exe 'vert 4resize ' . ((&columns * 84 + 127) / 254)
exe '5resize ' . ((&lines * 32 + 34) / 68)
exe 'vert 5resize ' . ((&columns * 84 + 127) / 254)
exe '6resize ' . ((&lines * 33 + 34) / 68)
exe 'vert 6resize ' . ((&columns * 84 + 127) / 254)
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
