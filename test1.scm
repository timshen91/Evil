(define a 1)
a

(define-syntax my
  (syntax-rules ()
                ((y x) x)))
(my 3)

(define (a) 3)
(a)
