import re
import ast
from z3 import *

def simplify_getelement(expression):
    pattern = r'getelement\(([^,]+),\s([^)]+)\)'
    def replacement(match):
        arg1, arg2 = match.group(1), match.group(2)
        return f'getelement_{arg1}_{arg2}'
    return re.sub(pattern, replacement, expression)

def simplify_getfield(expression):
    pattern = r'getfield\(([^,]+),\sgetelement\(([^,]+),\s([^)]+)\),\s([^)]+)\)'
    def replacement(match):
        arg1, arg2, arg3, arg4 = match.group(1), match.group(2), match.group(3), match.group(4)
        return f'getfield_{arg1}_getelement_{arg2}_{arg3}_{arg4}'
    return re.sub(pattern, replacement, expression)

def eval_expr(expr):
    if isinstance(expr, ast.BoolOp):
        if isinstance(expr.op, ast.And):
            return And(*(eval_expr(v) for v in expr.values))
        elif isinstance(expr.op, ast.Or):
            return Or(*(eval_expr(v) for v in expr.values))
    elif isinstance(expr, ast.BinOp):
        if isinstance(expr.op, ast.Mod):
            return eval_expr(expr.left) % eval_expr(expr.right)
        elif isinstance(expr.op, ast.Add):
            return eval_expr(expr.left) + eval_expr(expr.right)
        elif isinstance(expr.op, ast.Sub):
            return eval_expr(expr.left) - eval_expr(expr.right)
        elif isinstance(expr.op, ast.Mult):
            return eval_expr(expr.left) * eval_expr(expr.right)
        elif isinstance(expr.op, ast.Div):
            # For Z3, use ' / ' for floating point division or ' // ' for integer division if needed
            return eval_expr(expr.left) / eval_expr(expr.right)
    elif isinstance(expr, ast.Compare):
        left = eval_expr(expr.left)
        ops = expr.ops
        comparators = [eval_expr(comp) for comp in expr.comparators]
        if isinstance(ops[0], ast.NotEq):
            return left != comparators[0]
        elif isinstance(ops[0], ast.Eq):
            return left == comparators[0]
        elif isinstance(ops[0], ast.Gt):
            return left > comparators[0]
        elif isinstance(ops[0], ast.Lt):
            return left < comparators[0]
        elif isinstance(ops[0], ast.GtE):
            return left >= comparators[0]
        elif isinstance(ops[0], ast.LtE):
            return left <= comparators[0]
    elif isinstance(expr, ast.Num):
        return expr.n
    elif isinstance(expr, ast.Name):
        return Int(expr.id)
    elif isinstance(expr, ast.UnaryOp):
        if isinstance(expr.op, ast.USub):
            return -eval_expr(expr.operand)
        elif isinstance(expr.op, ast.UAdd):
            return +eval_expr(expr.operand)
    else:
        raise TypeError(f"Unsupported AST node type: {type(expr)}")

if __name__ == "__main__":
    precondition = sys.argv[1]
    program_state = sys.argv[2]

    if precondition.lower() == 'true':
        sys.exit(0)
    elif precondition.lower() == 'false':
        sys.exit(1)
    elif precondition.lower() == 'maybe':
        sys.exit(1)

    program_state = simplify_getfield(program_state)
    program_state = simplify_getelement(program_state)
    program_state = program_state.replace(' /\\ ', ' and ')
    program_state = program_state.replace(' = ', ' == ')

    precondition = simplify_getfield(precondition)
    precondition = simplify_getelement(precondition)
    precondition = precondition.replace(' /\\ ', ' and ')
    precondition = precondition.replace(' \\/ ', ' or ')
    precondition = precondition.replace(' = ', ' == ')
    precondition = precondition.replace(' div ', ' / ' )
    precondition = precondition.replace(' mod ', ' % ' )

    expr = "(" + precondition + ") and (" + program_state + ")"

    expr_ast = ast.parse(expr, mode='eval').body
    z3_expr = eval_expr(expr_ast)

    solver = Solver()
    solver.add(z3_expr)
    if solver.check() == sat:
        sys.exit(0)
    else:
        sys.exit(1)