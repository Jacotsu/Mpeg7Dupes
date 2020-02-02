def Settings( **kwargs ):
  return {
    'flags': [ '-x', 'c', '-Wall', '-Wextra', '-Werror', '-std=c99',
        '-I', 'src/includes', '-I', '/usr/include'],
  }
