

class Vec3:
    n = 3
    def __main__(self, name, **args):
        self.name = name
        n = self.n
        if 'min' in args:
            if len(args['min']) != 3:
                raise Exception(f'Vec{n} min must be {n} elements long.')
            self.min = args['min']
        if 'max' in args:
            if len(args['max']) != 3:
                raise Exception(f'Vec{n} max must be {n} elements long.')
            self.max = args['max']
        if 'val' in args:
            if len(args['val']) != 3:
                raise Exception(f'Vec{n} val must be {n} elements long.')
            self.val = args['val']
        if 'step' in args:
            if len(args['step']) != 3:
                raise Exception(f'Vec{n} step must be {n} elements long.')
            self.val = args['step']
    def __str__(self):
        return """
        """