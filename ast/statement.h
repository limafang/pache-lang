#ifndef STATEMENT_H
#define STATEMENT_H

#include "ast.h"
#include "expression.h"

#include <vector>

namespace pache {
  class stmt_ast : public base_ast {
  public:
    virtual std::string dump() override = 0;
    virtual ~stmt_ast() override { }
  };

  class block_ast : public stmt_ast {
  public:
    block_ast() = default;
    block_ast(block_ast && other) = default;
    block_ast(std::vector<stmt_ast*> *statements) : m_statements(statements) { }
    virtual void insert_dec(std::string name, variable_ast *dec) override {
      dec_name.insert(std::make_pair(name, dec));
    }

    virtual variable_ast * find_dec(std::string name) const override {
      auto beg = this->dec_name.find(name);
        if (beg != this->dec_name.end()) {
          return beg->second;
        } else if (m_father != nullptr) {
          return m_father->find_dec(name);
        } else {
          return nullptr;
        }
    }


    virtual std::string dump() override {
      std::cout << "\n";
      for (auto stmt : *m_statements) {
        // std::cout << "\t";
        std::cout << stmt->dump();
      }
      std::cout << "\n";
      return std::string{};
    }


    virtual ~block_ast() override = default;
  protected:
    std::vector<stmt_ast*> *m_statements;

    std::unordered_map<std::string, variable_ast*> dec_name;
  };

  class loop_stmt : public block_ast {
  public:
    explicit loop_stmt(block_ast *block) : m_block(block) { }

    virtual std::string dump() override {
      m_block->set_father(this);
      begin = std::to_string(ssa_value++);
      end = std::to_string(ssa_value++);
      std::cout << "\n";
      std::cout << begin <<  ":\n";
      m_block->dump();
      std::cout << "br lable %" << begin << "\n";
      std::cout << end_lable() << ":\n";
      std::cout << "\n";
      return std::string{};
    }
    virtual std::string begin_lable() const override {
      return begin;
    }

    virtual std::string end_lable() const  override {
      return end;
    }

  private:
    block_ast *m_block;
    std::string begin;
    std::string end;
  };

  class break_stmt : public stmt_ast {
  public:
    virtual std::string dump() override {
      return "br lable %" + end_lable() + "\n";
    }


  };

  class continue_stmt : public stmt_ast {
  public:
    virtual std::string dump() override {
      return "br lable %" + begin_lable() + "\n";
    }


  };

  class return_ast : public stmt_ast {
  public:
    explicit return_ast(exp_ast* exp) : m_exp(exp) { }
    virtual std::string dump() override {
      m_exp->set_father(m_father);
      std::string s1 = m_exp->dump();
      std::cout << "ret " << m_exp->get_type()->dump() << " " << s1 << "\n";
      return std::string{};
    }
    virtual ~return_ast() override = default;


  private:
    exp_ast* m_exp;
  };


  class assign_stmt : public stmt_ast {
  public:
    explicit assign_stmt(var_exp *var, exp_ast *val) : m_var(var), m_val(val) { }

    virtual std::string dump() override {
      m_var->set_father(m_father);
      m_val->set_father(m_father);
      variable_ast * var = m_var->get_var();
      std::string s = m_val->dump();
      std::cout << "store " << var->get_type()->dump()
                << " " << s << ", "
                << var->get_type()->dump() << "* @"
                << var->get_name() << "\n";

      return "";
    }
  private:
    var_exp *m_var;
    exp_ast *m_val;
  };

  class let_stmt : public stmt_ast {
  public:
    explicit let_stmt(variable_ast *var, exp_ast * init) : m_var(var), m_init(init) { }

    virtual std::string dump() override {
      std::string name = m_var->get_name();
      m_var->set_name(name + m_father->get_prefix());
      insert_dec(name, m_var);
      std::cout << "@" << m_var->get_name() << " = alloc " << m_var->get_type()->dump() << "\n";
      if (m_init != nullptr) {
        var_exp var(&(m_var->get_name()));
        assign_stmt assign(&var, m_init);
        assign.set_father(m_father);
        return assign.dump();
      } else {
        return "";
      }

    }

  private:
    variable_ast *m_var;
    exp_ast *m_init;
  };

  class exp_stmt : public stmt_ast {
  public:
    explicit exp_stmt(exp_ast *exp) : m_exp(exp) { }

    virtual std::string dump() override {
      m_exp->set_father(m_father);
      m_exp->dump();
      return "";
    }
  private:
    exp_ast *m_exp;
  };

  class for_ast : public stmt_ast {


  private:
    block_ast m_block;
    // range    TODO
    // var      TODO
  };

  class if_stmt : public block_ast {
  public:
    explicit if_stmt(exp_ast *exp, block_ast *then, block_ast *elses)
      : m_condition(exp), m_then_block(then), m_else_block(elses) { }

    virtual variable_ast * find_dec(std::string name) const override {
      return m_father->find_dec(name);
    }
    virtual std::string dump() override {
      m_condition->set_father(m_father);
      m_then_block->set_father(m_father);
      if (m_else_block != nullptr) {
        m_else_block->set_father(m_father);
      }
      std::string s1 = m_condition->dump();
      std::size_t block1 = ssa_value++;
      std::size_t block2 = ssa_value++;
      std::cout << "br i1 " << s1 << ", label %" << block1
                << ", label %" << block2 << "\n"
                << block1 << ":\n";
      std::string s2 = m_then_block->dump();
      std::size_t block3 = ssa_value++;
      std::cout << s2
                << "\nbr label %" << block3 << "\n\n"
                << block2 << ":\n";
      std::string s3 = m_else_block ? m_else_block->dump() : "";
      std::cout << s3
                << "\nbr label %" << block3
                << "\n\n" << block3 << ":\n";
      return "";
    }
  private:
    exp_ast *m_condition;
    block_ast *m_then_block;
    block_ast *m_else_block;
  };

}

#endif
