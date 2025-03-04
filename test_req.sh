curl -v http://localhost:8080/nova.mp4 \
  -H "X-Custom-Header-1: $(printf '%.3000s' $(printf 'A%.0s' {1..3000}))" \
  -H "X-Custom-Header-2: $(printf '%.3000s' $(printf 'A%.0s' {1..3000}))" \
  -H "X-Custom-Header-3: $(printf '%.3000s' $(printf 'A%.0s' {1..3000}))" \
  -H "X-Custom-Header-4: $(printf '%.3000s' $(printf 'A%.0s' {1..3000}))" \
  -H "X-Custom-Header-5: $(printf '%.3000s' $(printf 'A%.0s' {1..3000}))" --output ok.mp4