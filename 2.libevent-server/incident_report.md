# Crash

## Findings

- req to be freed is at 0xdfdfdfdfdfdfdfdf
- this pointer is non accessable
- when the req struct is calloc'd it is not memset'd or bzero'd
- maybe it is a garbage value
- when the link list is being constructed the first req struct next should be
  nulled
