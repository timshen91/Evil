(define-syntax my
  (syntax-rules (my)
                ((y x) (begin (display x)))))
