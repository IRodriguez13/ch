# Fish completion for ch-adds(1) and ch(1) alias

function __ch_adds_git_refs --description 'Git refs for ch-adds'
	printf '%s\n' HEAD develop
	if git rev-parse --git-dir >/dev/null 2>&1
		git for-each-ref --format='%(refname:short)' refs/heads refs/tags 2>/dev/null
	end
end

function __ch_adds_complete --description 'Shared ch-adds/ch completions'
	set -l cmd $argv[1]

	complete -c $cmd -s r -l ref -d 'Git reference or range' -xa '(__ch_adds_git_refs)'
	complete -c $cmd -s p -l patch -d 'Read from patch file' -rF
	complete -c $cmd -l staged -d 'Only staged changes'
	complete -c $cmd -l unstaged -d 'Only unstaged changes'
	complete -c $cmd -s 2 -l two-dot -d 'Use branch..HEAD'
	complete -c $cmd -s x -l exact -d 'Diff directly against REF'
	complete -c $cmd -s q -l quiet -d 'No header or line numbers'
	complete -c $cmd -s 0 -l plain -d 'With -q: omit +/- prefixes'
	complete -c $cmd -s d -l removed -d 'Also show removed lines'
	complete -c $cmd -s h -l help -d 'Show help'
	complete -c $cmd -s V -l version -d 'Show version and license'
	complete -c $cmd -F
end

__ch_adds_complete ch-adds
complete --wraps ch-adds -c ch
