# Fish completion for ch(1)

function __ch_git_refs --description 'Git refs for ch'
	if git rev-parse --git-dir >/dev/null 2>&1
		git for-each-ref --format='%(refname:short)' refs/heads refs/tags 2>/dev/null
	end
	echo HEAD
	echo develop
end

complete -c ch -s r -l ref -d 'Git reference or range' -xa '(__ch_git_refs)'
complete -c ch -s p -l patch -d 'Patch file' -rF
complete -c ch -l staged -d 'Only staged changes'
complete -c ch -l unstaged -d 'Only unstaged changes'
complete -c ch -s 2 -l two-dot -d 'Use branch..HEAD'
complete -c ch -s x -l exact -d 'Diff against REF exactly'
complete -c ch -s q -l quiet -d 'No header or line numbers'
complete -c ch -s 0 -l plain -d 'With -q, omit +/- prefix'
complete -c ch -s d -l removed -d 'Also show removed lines'
complete -c ch -s a -l all -d 'All changed files in repo'
complete -c ch -l diff -d 'Compare two directory trees' -xa '(__fish_complete_directories)'
complete -c ch -l exclude -d 'Skip paths in --diff' -x
complete -c ch -s h -l help -d 'Show help'
complete -c ch -s V -l version -d 'Show version'
complete -c ch -f -a '(__ch_git_refs)' -n '__fish_seen_subcommand_from ch; and not __fish_seen_subcommand_from -r --ref -p --patch --staged --unstaged -2 --two-dot -x --exact -q --quiet -0 --plain -d --removed -a --all -h --help -V --version'
