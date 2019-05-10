(begin
    (define make-point (lambda (x y) (begin (define a (set-property "object-name" "point" (list x y))) (define a (set-property "size" 0 a)))))
	(define make-line (lambda (p1 p2) (begin (define a (set-property "object-name" "line" (list p1 p2))) (define a (set-property "thickness" 1 a)))))
	(define make-text (lambda (str) (begin (define a (set-property "object-name" "text" (str))) (define origin (make-point 0 0)) (define a (set-property "position" origin a)) (define a (set-property "scale" 1 a)) (define a (set-property "rotation" 0 a)))))
)