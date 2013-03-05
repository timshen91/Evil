;(define a 1)
;a
;
;(define-syntax my
;  (syntax-rules ()
;                ((y x) x)))
;(my 3)
;
;(define (a) (+ 3 2))
;(a)
(define (a x)
  (if (= x 0) 1 (* x (a (- x 1)))))
(a 10)
