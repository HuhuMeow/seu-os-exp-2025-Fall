let s:cpo_save=&cpo
set cpo&vim
cnoremap <silent> <Plug>(TelescopeFuzzyCommandSearch) e "lua require('telescope.builtin').command_history { default_text = [=[" . escape(getcmdline(), '"') . "]=] }"
inoremap <C-W> u
inoremap <C-U> u
nnoremap <silent>  :TmuxNavigateLeft
nnoremap <silent> <NL> :TmuxNavigateDown
nnoremap <silent>  :TmuxNavigateUp
nnoremap <silent>  :TmuxNavigateRight
nmap  d
nnoremap <silent>  :TmuxNavigatePrevious
nnoremap  e :NvimTreeToggle
nnoremap  h :bprevious
nnoremap  l :bnext
nnoremap  sv s
nnoremap  sh v
omap <silent> % <Plug>(MatchitOperationForward)
xmap <silent> % <Plug>(MatchitVisualForward)
nmap <silent> % <Plug>(MatchitNormalForward)
nnoremap & :&&
xnoremap <silent> <expr> @ mode() ==# 'V' ? ':normal! @'.getcharstr().'' : '@'
vnoremap <silent> C "_C
nnoremap <silent> C "_C
vnoremap J :m '>+1gv=gv
vnoremap K :m '<-2gv=gv
xnoremap <silent> <expr> Q mode() ==# 'V' ? ':normal! @=reg_recorded()' : 'Q'
nnoremap Y y$
omap <silent> [% <Plug>(MatchitOperationMultiBackward)
xmap <silent> [% <Plug>(MatchitVisualMultiBackward)
nmap <silent> [% <Plug>(MatchitNormalMultiBackward)
omap <silent> ]% <Plug>(MatchitOperationMultiForward)
xmap <silent> ]% <Plug>(MatchitVisualMultiForward)
nmap <silent> ]% <Plug>(MatchitNormalMultiForward)
xmap a% <Plug>(MatchitVisualTextObject)
vnoremap <silent> c "_c
nnoremap <silent> c "_c
nnoremap <silent> dd "_dd
vnoremap <silent> d "_d
nnoremap <silent> d "_d
xnoremap gb <Plug>(comment_toggle_blockwise_visual)
nnoremap gb <Plug>(comment_toggle_blockwise)
omap <silent> g% <Plug>(MatchitOperationBackward)
xmap <silent> g% <Plug>(MatchitVisualBackward)
nmap <silent> g% <Plug>(MatchitNormalBackward)
xnoremap gc <Plug>(comment_toggle_linewise_visual)
nnoremap gc <Plug>(comment_toggle_linewise)
xnoremap p P
xmap <silent> <Plug>(MatchitVisualTextObject) <Plug>(MatchitVisualMultiBackward)o<Plug>(MatchitVisualMultiForward)
onoremap <silent> <Plug>(MatchitOperationMultiForward) :call matchit#MultiMatch("W",  "o")
onoremap <silent> <Plug>(MatchitOperationMultiBackward) :call matchit#MultiMatch("bW", "o")
xnoremap <silent> <Plug>(MatchitVisualMultiForward) :call matchit#MultiMatch("W",  "n")m'gv``
xnoremap <silent> <Plug>(MatchitVisualMultiBackward) :call matchit#MultiMatch("bW", "n")m'gv``
nnoremap <silent> <Plug>(MatchitNormalMultiForward) :call matchit#MultiMatch("W",  "n")
nnoremap <silent> <Plug>(MatchitNormalMultiBackward) :call matchit#MultiMatch("bW", "n")
onoremap <silent> <Plug>(MatchitOperationBackward) :call matchit#Match_wrapper('',0,'o')
onoremap <silent> <Plug>(MatchitOperationForward) :call matchit#Match_wrapper('',1,'o')
xnoremap <silent> <Plug>(MatchitVisualBackward) :call matchit#Match_wrapper('',0,'v')m'gv``
xnoremap <silent> <Plug>(MatchitVisualForward) :call matchit#Match_wrapper('',1,'v'):if col("''") != col("$") | exe ":normal! m'" | endifgv``
nnoremap <silent> <Plug>(MatchitNormalBackward) :call matchit#Match_wrapper('',0,'n')
nnoremap <silent> <Plug>(MatchitNormalForward) :call matchit#Match_wrapper('',1,'n')
xnoremap <Plug>(comment_toggle_blockwise_visual) <Cmd>lua require("Comment.api").locked("toggle.blockwise")(vim.fn.visualmode())
xnoremap <Plug>(comment_toggle_linewise_visual) <Cmd>lua require("Comment.api").locked("toggle.linewise")(vim.fn.visualmode())
nnoremap <silent> <C-Bslash> :TmuxNavigatePrevious
nnoremap <silent> <C-K> :TmuxNavigateUp
nnoremap <silent> <C-J> :TmuxNavigateDown
nnoremap <silent> <C-H> :TmuxNavigateLeft
nnoremap <Plug>PlenaryTestFile :lua require('plenary.test_harness').test_file(vim.fn.expand("%:p"))
nmap <C-W><C-D> d
nnoremap <silent> <C-L> :TmuxNavigateRight
inoremap  u
inoremap  u
let &cpo=s:cpo_save
unlet s:cpo_save
set clipboard=unnamedplus
set expandtab
set grepformat=%f:%l:%c:%m
set grepprg=rg\ --vimgrep\ -uu\ 
set helplang=en
set ignorecase
set noloadplugins
set mouse=nvia
set packpath=/opt/homebrew/Cellar/neovim/0.11.4/share/nvim/runtime
set runtimepath=~/.config/nvim,~/.local/share/nvim/lazy/lazy.nvim,~/.local/share/nvim/lazy/nvim-cmp,~/.local/share/nvim/lazy/cmp-path,~/.local/share/nvim/lazy/friendly-snippets,~/.local/share/nvim/lazy/nvim-tree.lua,~/.local/share/nvim/lazy/lualine.nvim,~/.local/share/nvim/lazy/Comment.nvim,~/.local/share/nvim/lazy/LuaSnip,~/.local/share/nvim/lazy/nvim-treesitter,~/.local/share/nvim/lazy/cmp_luasnip,~/.local/share/nvim/lazy/nvim-web-devicons,~/.local/share/nvim/lazy/vim-tmux-navigator,~/.local/share/nvim/lazy/tokyonight.nvim,~/.local/share/nvim/lazy/nvim-lspconfig,~/.local/share/nvim/lazy/vimtex,~/.local/share/nvim/lazy/gitsigns.nvim,~/.local/share/nvim/lazy/mason-lspconfig.nvim,~/.local/share/nvim/lazy/mason.nvim,~/.local/share/nvim/lazy/nvim-autopairs,~/.local/share/nvim/lazy/plenary.nvim,~/.local/share/nvim/lazy/telescope.nvim,~/.local/share/nvim/lazy/rainbow-delimiters.nvim,~/.local/share/nvim/lazy/bufferline.nvim,~/.local/share/nvim/lazy/cmp-nvim-lsp,/opt/homebrew/Cellar/neovim/0.11.4/share/nvim/runtime,/opt/homebrew/Cellar/neovim/0.11.4/share/nvim/runtime/pack/dist/opt/netrw,/opt/homebrew/Cellar/neovim/0.11.4/share/nvim/runtime/pack/dist/opt/matchit,/opt/homebrew/Cellar/neovim/0.11.4/lib/nvim,~/.local/state/nvim/lazy/readme,~/.local/share/nvim/lazy/cmp-path/after,~/.local/share/nvim/lazy/cmp_luasnip/after,~/.local/share/nvim/lazy/vimtex/after,~/.local/share/nvim/lazy/mason-lspconfig.nvim/after,~/.local/share/nvim/lazy/cmp-nvim-lsp/after
set shiftwidth=2
set showtabline=2
set smartcase
set splitbelow
set splitright
set statusline=%#lualine_transparent#
set suffixes=.bak,~,.o,.h,.info,.swp,.obj,.sty,.cls,.log,.aux,.bbl,.out,.blg,.brf,.cb,.dvi,.fdb_latexmk,.fls,.idx,.ilg,.ind,.inx,.pdf,.synctex.gz,.toc
set tabline=%!v:lua.nvim_bufferline()
set tabstop=2
set termguicolors
set window=38
" vim: set ft=vim :
