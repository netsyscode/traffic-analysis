#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <json.hpp>

class SVMPredictor {
private:
    std::vector<std::vector<std::vector<double>>> support_vectors;
    std::vector<std::vector<std::vector<double>>> dual_coef;
    std::vector<double> intercept;
    std::vector<double> mean;  // 新增
    std::vector<double> std;   // 新增
    double gamma = 1.0/24.0;

    double rbf_kernel(const std::vector<double>& x1, const std::vector<double>& x2) const {
        double squared_distance = 0.0;
        for (size_t i = 0; i < x1.size(); ++i) {
            squared_distance += (x1[i] - x2[i]) * (x1[i] - x2[i]);
        }
        return exp(-gamma * squared_distance);
    }

public:
    SVMPredictor(const std::string& jsonFilePath, double gammaValue) : gamma(gammaValue) {
        // 读取 JSON 文件
        std::ifstream file(jsonFilePath);
        nlohmann::json model_params;
        file >> model_params;
       support_vectors = model_params["support_vectors"].get<std::vector<std::vector<std::vector<double>>>>();
	dual_coef = model_params["dual_coef"].get<std::vector<std::vector<std::vector<double>>>>();
        intercept = model_params["intercept"].get<std::vector<double>>();
        mean = model_params["mean"].get<std::vector<double>>();
        std = model_params["std"].get<std::vector<double>>();
        std::cout << "Number of classes (from intercept): " << intercept.size() << std::endl;
	std::cout << "Number of support vector sets: " << support_vectors.size() << std::endl;
	if(!support_vectors.empty()) {
	    std::cout << "Number of support vectors in first set: " << support_vectors[0].size() << std::endl;
	    if(!support_vectors[0].empty()) {
		std::cout << "Dimension of first support vector in first set: " << support_vectors[0][0].size() << std::endl;
	    }
	}
	std::cout << "Number of dual coef sets: " << dual_coef.size() << std::endl;
	if(!dual_coef.empty()) {
	    std::cout << "Number of dual coef in first set: " << dual_coef[0].size() << std::endl;
	    if(!dual_coef[0].empty()) {
		std::cout << "Dimension of first dual coef in first set: " << dual_coef[0][0].size() << std::endl;
	    }
	}
	std::cout << "Dimension of mean: " << mean.size() << std::endl;
	std::cout << "Dimension of std: " << std.size() << std::endl;
    }
    std::vector<double> standardize(const std::vector<double>& x) const {
        std::vector<double> result(x.size());
        for (size_t i = 0; i < x.size(); ++i) {
            if(std[i] != 0)
            	result[i] = (x[i] - mean[i]) / std[i];
            else
            	result[i] = x[i];
        }
        return result;
    }
    int predict(const std::vector<double>& x) const {
    	std::vector<double> standardized_x  = standardize(x);  // 使用标准化
        int num_classes = intercept.size();
        std::vector<double> scores(num_classes, 0.0);

        for (int i = 0; i < num_classes; ++i) {
        for (size_t j = 0; j < support_vectors[i].size(); ++j) {
                scores[i] += dual_coef[i][0][j] * rbf_kernel(standardized_x , support_vectors[i][j]);
        }
            scores[i] += intercept[i];
        }

        return std::distance(scores.begin(), std::max_element(scores.begin(), scores.end()));
    }
    
    std::pair<int, std::vector<double>> predictWithDistances(const std::vector<double>& x) const {
    	std::vector<double> standardized_x = standardize(x);  // 使用标准化
        int num_classes = intercept.size();
        std::vector<double> scores(num_classes, 0.0);

        for (int i = 0; i < num_classes; ++i) {
        for (size_t j = 0; j < support_vectors[i].size(); ++j) {
                scores[i] += dual_coef[i][0][j]  * rbf_kernel(standardized_x , support_vectors[i][j]);
        }
            scores[i] += intercept[i];
        }
        int predicted_class = std::distance(scores.begin(), std::max_element(scores.begin(), scores.end()));
        return {predicted_class, scores};
    }

    std::pair<int, double> predictForPredictedClass(const std::vector<double>& x) const {
    	std::vector<double> standardized_x = standardize(x);  // 特征使用标准化
        int num_classes = intercept.size();
        std::vector<double> scores(num_classes, 0.0);
        for (int i = 0; i < num_classes; ++i) {
            for (size_t j = 0; j < support_vectors[i].size(); ++j) {
                    scores[i] += dual_coef[i][0][j]  * rbf_kernel(standardized_x , support_vectors[i][j]);
            }
            scores[i] += intercept[i];
        }
        int predicted_class = std::distance(scores.begin(), std::max_element(scores.begin(), scores.end()));
        return {predicted_class, scores[predicted_class]};
    }

    void displayPredictionWithDistances(const std::vector<double>& x) const {
        auto [predicted_class, distances] = predictWithDistances(x);
        std::cout << "Predicted class: " << predicted_class << std::endl;
        std::cout << "Distances to decision boundaries: ";
        for (double distance : distances) {
            std::cout << distance << " ";
        }
        std::cout << std::endl;
    }
};
